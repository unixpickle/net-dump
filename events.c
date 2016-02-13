#include "events.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int is_data_packet(const u_char * macPacket, size_t len);
static client_event * read_mac_info(const u_char * macPacket, size_t len);
static int radiotap_rssi(const u_char * header);
static void print_hardware_address(hardware_address a);
static void print_csv_escaped(const char * str);

client_event * client_event_read(pcap_t * handle) {
  while (1) {
    struct pcap_pkthdr * header;
    const u_char * buffer;
    int ret = pcap_next_ex(handle, &header, &buffer);
    if (ret < 0) {
      return NULL;
    }
    if (ret != 1 || header->caplen < 4) {
      continue;
    }

    size_t radiotapLen = (size_t)((uint16_t *)buffer)[1];
    if (header->caplen < radiotapLen) {
      continue;
    }

    const u_char * macPacket = buffer + radiotapLen;
    size_t macPacketLen = (size_t)header->caplen - radiotapLen;
    if (!is_data_packet(macPacket, macPacketLen)) {
      continue;
    }

    client_event * res = read_mac_info(macPacket, macPacketLen);
    res->timestamp = header->ts;
    res->rssi = radiotap_rssi(buffer);
    res->request_info = http_req_in_packet(macPacket, macPacketLen);
    return res;
  }
}

void client_event_log_csv(client_event * e) {
  switch (e->type) {
  case CLIENT_EVENT_TO_AP:
    printf("to_ap");
    break;
  case CLIENT_EVENT_FROM_AP:
    printf("from_ap");
    break;
  case CLIENT_EVENT_INTER_HOST:
    printf("inter_host");
    break;
  default:
    printf("unknown_type");
    break;
  }
  printf(",%llu,%d,", (unsigned long long)e->timestamp.tv_sec, e->rssi);
  print_hardware_address(e->client);
  printf(",");
  print_hardware_address(e->accessPoint);
  if (e->request_info != NULL) {
    printf(",");
    print_csv_escaped(e->request_info->path);
    printf(",");
    print_csv_escaped(e->request_info->host);
    printf(",");
    print_csv_escaped(e->request_info->user_agent);
    printf("\n");
  } else {
    printf(",,,\n");
  }
  fflush(stdout);
}

void client_event_free(client_event * e) {
  if (e->request_info != NULL) {
    http_req_free(e->request_info);
  }
  free(e);
}

static int is_data_packet(const u_char * macPacket, size_t len) {
  if (len < 22) {
    return 0;
  }
  int majorType = (int)(macPacket[0] >> 2) & 3;
  return majorType == 2;
}

static client_event * read_mac_info(const u_char * macPacket, size_t len) {
  client_event * event = (client_event *)malloc(sizeof(client_event));

  int toDS = (int)(macPacket[1] & 1);
  int fromDS = (int)(macPacket[1] & 2);

  if (!fromDS && toDS) {
    memcpy(event->accessPoint, macPacket+4, 6);
    memcpy(event->client, macPacket+10, 6);
    event->type = CLIENT_EVENT_TO_AP;
  } else if (!fromDS && !toDS) {
    memcpy(event->accessPoint, macPacket+16, 6);
    memcpy(event->client, macPacket+10, 6);
    event->type = CLIENT_EVENT_INTER_HOST;
  } else if (fromDS && !toDS) {
    memcpy(event->accessPoint, macPacket+10, 6);
    memcpy(event->client, macPacket+4, 6);
    event->type = CLIENT_EVENT_FROM_AP;
  }

  return event;
}

static int radiotap_rssi(const u_char * packet) {
    u_char present = packet[4];
    if (!(present & 0x20)) {
        return 0;
    }
    size_t fieldOffset = 0;
    if (present & 1) {
        fieldOffset += 8;
    }
    if (present & 2) {
        fieldOffset += 1;
    }
    if (present & 4) {
        fieldOffset += 1;
    }
    if (present & 8) {
        if (fieldOffset & 1) {
            fieldOffset++;
        }
        fieldOffset += 4;
    }
    if (present & 0x10) {
        if (fieldOffset & 1) {
            fieldOffset++;
        }
        fieldOffset += 2;
    }

    return (int)((char *)packet)[8+fieldOffset];
}

static void print_hardware_address(hardware_address a) {
  printf("%02x:%02x:%02x:%02x:%02x:%02x", a[0], a[1], a[2], a[3], a[4], a[5]);
}

static void print_csv_escaped(const char * str) {
  if (str == NULL) {
    return;
  }
  int len = strlen(str);
  int i;
  for (i = 0; i < len; ++i) {
    if (str[i] == '\\' || str[i] == ',') {
      printf("\\%c", str[i]);
    } else {
      putc(str[i], stdout);
    }
  }
}

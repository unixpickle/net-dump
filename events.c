#include "events.h"

static int is_data_packet(const u_char * macPacket, size_t len);
static client_event * read_mac_info(const u_char * macPacket, size_t len);
static int radiotap_rssi(const u_char * header);

client_event * client_event_read(pcap_t * handle) {
  while (true) {
    struct pcap_pkthdr * header;
    const u_char * buffer;
    int ret = pcap_next_ex(pcapHandle, &header, &buffer);
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

    client_info * res = read_mac_info(macPacket, macPacketLen);
    res.timestamp = header.ts;
    res.rssi = radiotap_rssi(buffer);
    res.request_info = http_req_in_packet(macPacket, macPacketLen);
    return res;
  }
}

void client_event_free(client_event * e) {
  if (e.request_info != NULL) {
    http_req_free(e.request_info);
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

  int minor = (int)(macPacket[0] >> 4) & 0xf;
  int toDS = (int)(macPacket[1] & 1);
  int fromDS = (int)(macPacket[1] & 2);

  if (!fromDS && toDS) {
    memcpy(event.accessPoint, macPacket+4, 6);
    memcpy(event.client, macPacket+10, 6);
    event.type = CLIENT_EVENT_TO_AP;
  } else if (!fromDS && !toDS) {
    memcpy(event.accessPoint, macPacket+16, 6);
    memcpy(event.client, macPacket+10, 6);
    event.type = CLIENT_EVENT_INTER_HOST;
  } else if (fromDS && !toDS) {
    memcpy(event.accessPoint, macPacket+10, 6);
    memcpy(event.client, macPacket+4, 6);
    event.type = CLIENT_EVENT_FROM_AP;
  }
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

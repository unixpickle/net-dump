#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#include "channel.h"
#include "die.h"
#include "events.h"

const int DEFAULT_HOP_INTERVAL = 5;

static int get_channel_list(const char ** strList, int count, int * channels);

int main(int argc, const char ** argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <interface> [-h hop_interval] <channel> [channel ...]\n", argv[0]);
    return 1;
  }

  int channelsStartIndex = 2;
  int hopInterval = DEFAULT_HOP_INTERVAL;
  if (argc >= 5) {
    if (strcmp(argv[2], "-h") == 0) {
      hopInterval = atoi(argv[3]);
      if (hopInterval == 0) {
        fprintf(stderr, "Invalid hop interval: %s\n", argv[3]);
        return 1;
      }
      channelsStartIndex = 4;
    }
  }

#ifdef __APPLE__
  // On OS X, we must explicitly disassociate from WiFi before sniffing packets.
  system("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/"
    "Resources/airport -z");
#endif

  char errBuff[PCAP_ERRBUF_SIZE];
  pcap_t * pcapHandle = pcap_open_live(argv[1], 65536, 1, 100, errBuff);
  if (pcapHandle == NULL) {
    fprintf(stderr, "pcap error: %s\n", errBuff);
    return 1;
  }
  if (pcap_set_datalink(pcapHandle, DLT_IEEE802_11_RADIO) != 0) {
    fprintf(stderr, "pcap error: %s\n", pcap_geterr(pcapHandle));
    pcap_close(pcapHandle);
    return 1;
  }

  int * channels = (int *)malloc(sizeof(int)*(argc-channelsStartIndex));
  if (get_channel_list(argv+channelsStartIndex, argc-channelsStartIndex, channels) < 0) {
    fprintf(stderr, "invalid channel list.\n");
    return 1;
  }
  channel_hop_info * hopInfo = (channel_hop_info *)malloc(sizeof(channel_hop_info));
  hopInfo->channels = channels;
  hopInfo->count = argc - 2;
  hopInfo->interface = argv[1];
  hopInfo->hopDelay = hopInterval;
  hop_channels_async(hopInfo);

  while (1) {
    client_event * e = client_event_read(pcapHandle);
    if (e == NULL) {
      die("failed to read next event.");
    }
    client_event_log_csv(e);
    client_event_free(e);
  }
}

static int get_channel_list(const char ** strList, int count, int * channels) {
  int i;
  for (i = 0; i < count; i++) {
    channels[i] = atoi(strList[i]);
    if (!channels[i]) {
      return -1;
    }
  }
  return 0;
}

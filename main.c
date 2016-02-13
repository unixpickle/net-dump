#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

#include "channel.h"
#include "die.h"
#include "events.h"

const int CHANNEL_HOP_SECONDS = 5;

static int get_channel_list(int argc, const char ** argv, int * channels);

int main(int argc, const char ** argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <interface> <channel> [channel ...]\n", argv[0]);
    return 1;
  }

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

  int * channels = (int *)malloc(sizeof(int)*(argc-2));
  if (get_channel_list(argc, argv, channels) < 0) {
    fprintf(stderr, "invalid channel list.\n");
    return 1;
  }
  channel_hop_info * hopInfo = (channel_hop_info *)malloc(sizeof(channel_hop_info));
  hopInfo->channels = channels;
  hopInfo->count = argc - 2;
  hopInfo->interface = argv[1];
  hopInfo->hopDelay = CHANNEL_HOP_SECONDS;
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

static int get_channel_list(int argc, const char ** argv, int * channels) {
  int i;
  for (i = 2; i < argc; i++) {
    channels[i-2] = atoi(argv[i]);
    if (!channels[i-2]) {
      return -1;
    }
  }
  return 0;
}

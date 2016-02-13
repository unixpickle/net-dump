#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

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

  free(channels);
}

static int get_channel_list(int argc, const char ** argv, int * channels) {
  for (int i = 2; i < argc; i++) {
    channels[i-2] = atoi(argv[i]);
    if (!channels[i-2]) {
      return -1;
    }
  }
  return 0;
}

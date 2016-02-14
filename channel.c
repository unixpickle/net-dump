#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "channel.h"
#include "die.h"

static void * hop_thread(void * info);

int switch_channel(const char * interface, int channel) {
  if (strlen(interface) > 16 || channel <= 0 || channel > 999) {
    return -1;
  }

#ifdef __APPLE__
  char command[128];
  snprintf(command, sizeof(command), "/System/Library/PrivateFrameworks/Apple80211.framework/"
    "Versions/Current/Resources/airport -c%d", channel);
#else
  // TODO: look into using libiw here.
  char command[64];
  snprintf(command, sizeof(command), "iwconfig %s channel %d", interface, channel);
#endif

  return system(command);
}

void hop_channels_async(const channel_hop_info * list) {
  pthread_t thread;
  pthread_create(&thread, NULL, hop_thread, (void *)list);
}

static void * hop_thread(void * info) {
  const channel_hop_info * hopInfo = (const channel_hop_info *)info;
  int idx = 0;
  while (1) {
    int channel = hopInfo->channels[idx];
    if (switch_channel(hopInfo->interface, channel)) {
      die("failed to hop channels.");
    }
    idx = (idx + 1) % hopInfo->count;
    sleep(hopInfo->hopDelay);
  }
  return NULL;
}

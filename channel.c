#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "channel.h"
#include "die.h"

static void * hop_thread(void * info);

int switch_channel(const char * interface, const char * channel) {
  if (strlen(interface) > 16 || strlen(channel) > 9) {
    return -1;
  }

#ifdef __APPLE__
  if (atoi(channel) == 0) {
    return -1;
  }
  char command[128];
  snprintf(command, sizeof(command), "/System/Library/PrivateFrameworks/Apple80211.framework/"
    "Versions/Current/Resources/airport -c%s", channel);
#else
  // TODO: look into using libiw here.
  char command[64];
  const char * useIW = getenv("USE_IW");
  if (useIW != NULL && strcmp(useIW, "1") == 0) {
    // TODO: validate that the channel string is formatted correctly.
    snprintf(command, sizeof(command), "iw dev %s set channel %s", interface, channel);
  } else {
    if (atoi(channel) == 0) {
      return -1;
    }
    snprintf(command, sizeof(command), "iwconfig %s channel %s", interface, channel);
  }
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
    const char * channel = hopInfo->channels[idx];
    if (switch_channel(hopInfo->interface, channel)) {
      die("failed to hop channels.");
    }
    idx = (idx + 1) % hopInfo->count;
    sleep(hopInfo->hopDelay);
  }
  return NULL;
}

typedef struct {
  const char * interface;
  const int * channels;
  int count;
  int hopDelay;
} channel_hop_info;

// switch_channel synchronously switches WLAN channels.
int switch_channel(const char * interface, int channel);

// hop_channels_async asynchronously hops between channels in an infinite loop.
void hop_channels_async(const channel_hop_info * list);

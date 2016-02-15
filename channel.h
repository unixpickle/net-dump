typedef struct {
  const char * interface;
  const char ** channels;
  int count;
  int hopDelay;
} channel_hop_info;

// switch_channel synchronously switches WLAN channels.
// The channel argument may be a string representation of the channel number, or a string of the
// form "x HTy" where x is a channel number and y is 20, 40+, or 40-, indicating the channel width.
int switch_channel(const char * interface, const char * channel);

// hop_channels_async asynchronously hops between channels in an infinite loop.
void hop_channels_async(const channel_hop_info * list);

#include <stdlib.h>

int switch_channel(const char * interface, int channel) {
  if (strlen(interface) > 16 || channel <= 0 || channel > 999) {
    return -1;
  }

  // TODO: look into using libiw here.
  char command[64];
  snprintf(command, sizeof(command), "iwconfig %s channel %d", interface, channel)
  return system(command);
}

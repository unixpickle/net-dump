#include "http_req.h"
#include <pcap.h>

typedef char hardware_address[6];

typedef enum {
  CLIENT_EVENT_TO_AP,
  CLIENT_EVENT_FROM_AP,
  CLIENT_EVENT_INTER_HOST,
} client_event_type;

typedef struct {
  client_event_type type;

  struct timeval   timestamp;
  int              rssi;
  hardware_address client;
  hardware_address accessPoint;

  http_req * request_info;
} client_event;

// read_next_event reads packets from the pcap handle until it finds a client event.
// It allocates the client_event and returns it.
// It returns NULL if a packet cannot be read.
client_event * client_event_read(pcap_t * handle);

// client_event_log_csv prints the CSV representation of a client_event to stdout.
void client_event_log_csv(client_event * e);

// client_event_free deallocates a client_event from client_event_read().
void client_event_free(client_event * e);

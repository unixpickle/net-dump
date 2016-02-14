#include <stddef.h>

typedef struct {
  char method[5];
  char * path;
  char * host;
  char * user_agent;
} http_req;

// http_req_in_packet reads a packet and sees if it is likely to contain an HTTP request.
//
// If the packet does contain an HTTP request, this parses the request and returns information about
// it (which the caller must free).
//
// If the packet does not contain an HTTP request, this returns NULL.
http_req * http_req_in_packet(const void * packet, size_t len);

// http_req_free deallocates an http_req and all of its fields.
void http_req_free(http_req * r);

typedef struct {
  int     header_count;
  char ** header_names;
  char ** header_values;
} http_resp;

// http_resp_in_packet reads a packet and sees if it is likely to contain an HTTP response.
//
// If the packet does contain an HTTP response, this parses the response and returns information
// about it (which the caller must free).
//
// This will return NULL if the packet does not contain an HTTP response.
http_resp * http_resp_in_packet(const void * packet, size_t len);

// http_resp_free deallocates an http_resp and all of its fields.
void http_resp_free(http_resp * r);

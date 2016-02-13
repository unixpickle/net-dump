typedef struct {
  char method[5];
  char * path;
  char * host;
  char * userAgent;
} http_req;

// http_req_in_packet reads a packet and sees if it is likely to contain an HTTP request.
//
// If the packet does contain an HTTP request, this parses the request and returns information about
// it (which the caller must free).
//
// If the packet does not contain an HTTP request, this returns NULL.
http_req * http_req_in_packet(const void * packet, size_t len);

// http_req_free deallocates a http_req and all of its fields.
void http_req_free(http_req * r);

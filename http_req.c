#include "http_req.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static char * find_http_header(const char * packet, size_t len, const char * header);
static int string_has_header(const char * str, const char * header);
static ssize_t substr_index(const char * haystack, size_t len, const char * needle);
static ssize_t substr_index_before(const char * haystack, size_t len, const char * needle,
                                   ssize_t start);

http_req * http_req_in_packet(const void * packet, size_t len) {
  const char * packetStr = (const char *)packet;
  ssize_t httpIndex = substr_index(packetStr, len, "HTTP/1.");
  if (httpIndex < 0) {
    return NULL;
  }

  char * reqMethod = NULL;
  char * reqMethods[] = {"POST", "GET", "PUT", "HEAD"};
  ssize_t pathStartIndex = -1;
  size_t i;
  for (i = 0; i < sizeof(reqMethods)/sizeof(char *); ++i) {
    pathStartIndex = substr_index_before(packetStr, len, reqMethods[i], httpIndex);
    if (pathStartIndex >= 0) {
      reqMethod = reqMethods[i];
      pathStartIndex += strlen(reqMethod)+1;
      break;
    }
  }

  if (reqMethod == NULL) {
    return NULL;
  }

  ssize_t pathLen = (httpIndex - 1) - pathStartIndex;
  if (pathLen <= 0) {
    return NULL;
  }

  http_req * req = (http_req *)malloc(sizeof(http_req));
  bzero(req, sizeof(http_req));
  strncpy(req->method, reqMethod, sizeof(req->method)-1);
  req->path = (char *)malloc(pathLen+1);
  req->path[pathLen] = 0;
  memcpy(req->path, packetStr+pathStartIndex, pathLen);

  req->host = find_http_header(packetStr+httpIndex, len-httpIndex, "host");
  req->user_agent = find_http_header(packetStr+httpIndex, len-httpIndex, "user-agent");

  return req;
}

void http_req_free(http_req * r) {
  free(r->path);
  if (r->host != NULL) {
    free(r->host);
  }
  if (r->user_agent != NULL) {
    free(r->user_agent);
  }
  free(r);
}

static char * find_http_header(const char * packet, size_t len, const char * header) {
  char * lineBuffer = (char *)malloc(len+1);
  bzero(lineBuffer, len+1);

  size_t i;
  for (i = 0; i < len+1; i++) {
    char ch = 0;
    if (i < len) {
      ch = packet[i];
    }
    if (ch == 0 || ch == '\n' || ch == '\r') {
      if (string_has_header(lineBuffer, header)) {
        size_t headerStart = strlen(header) + 2;
        size_t lineLen = strlen(lineBuffer);
        memmove(lineBuffer, lineBuffer+headerStart, lineLen-headerStart);
        lineBuffer[lineLen - headerStart] = 0;
        return lineBuffer;
      }
      bzero(lineBuffer, len+1);
    } else {
      lineBuffer[strlen(lineBuffer)] = ch;
    }
  }

  free(lineBuffer);
  return NULL;
}

static int string_has_header(const char * str, const char * header) {
  size_t len1 = strlen(str);
  size_t len2 = strlen(header);
  if (len1 < len2+2) {
    return 0;
  }
  size_t i;
  for (i = 0; i < len2; i++) {
    if (tolower(str[i]) != tolower(header[i])) {
      return 0;
    }
  }
  if (str[len2] != ':' || str[len2+1] != ' ') {
    return 0;
  }
  return 1;
}

static ssize_t substr_index(const char * haystack, size_t len, const char * needle) {
  size_t needleLen = strlen(needle);
  return substr_index_before(haystack, len, needle, (ssize_t)(len-needleLen));
}

static ssize_t substr_index_before(const char * haystack, size_t len, const char * needle,
                                      ssize_t start) {
  size_t needleLen = strlen(needle);
  ssize_t i;
  for (i = start; i >= 0; --i) {
    if (memcmp(haystack+i, needle, needleLen) == 0) {
      return i;
    }
  }
  return -1;
}

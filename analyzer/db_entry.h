#ifndef __DB_ENTRY_H__
#define __DB_ENTRY_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char *             type;
  unsigned long long timestamp;
  int                rssi;
  size_t             size;
  char *             client;
  char *             access_point;

  char *             request_path;
  char *             request_host;
  char *             request_user_agent;

  char *             response_headers;
} db_entry;

typedef struct {
  db_entry * entries;
  int        count;
} db;

// db_read attempts to read a database line by line from a file handle.
//
// If the read fails, this returns NULL.
// If this returns a non-NULL pointer, you must free it with db_free().
db * db_read(FILE * f);

// db_free deallocates the result from db_read().
void db_free(db * d);

#endif

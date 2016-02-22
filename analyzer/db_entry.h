#ifndef __DB_ENTRY_H__
#define __DB_ENTRY_H__

#include <stdio.h>

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

// db_entry_read parses the next entry in a CSV file.
//
// If EOF is reached, this returns -1.
// If any other occurs, this returns 1.
// If this succeeds to read an entry, the entries fields will be set on *e, and this will return 0.
//
// Upon success, the fields of the returned entry should be freed with db_entry_free_fields().
int db_entry_read(FILE * f, db_entry * e);

// db_entry_free_fields deallocates the fields of a db_entry.
void db_entry_free_fields(db_entry * e);

#endif

#ifndef __DB_H__
#define __DB_H__

#include "db_entry.h"
#include "flags.h"

// A db_filter specifies various restrictions on a database's entries.
typedef struct {
  // clientMAC is the string representation of a client's MAC.
  // Valid values are 17 characters long (e.g. "aa:bb:cc:dd:ee").
  // If this filter should not check client MACs, set this field to "".
  char clientMAC[18];

  // startTime is an epoch time indicating the first allowed timestamp.
  // If this is 0, no lower bound is used.
  unsigned long long startTime;

  // endTime is an epoch time indicating the last allowed timestamp.
  // If this is 0, no upper bound is used.
  unsigned long long endTime;
} db_filter;

db_filter db_filter_from_flags(cmd_flags * flags);

// db_filter_matches returns 1 if the entry matches the filter, or 0 otherwise.
int db_filter_matches(db_filter f, db_entry * e);

typedef struct {
  db_entry * entries;
  int        count;
} db;

// db_read attempts to read a database line by line from a file handle.
//
// If the read fails, this returns NULL.
// If this returns a non-NULL pointer, you must free it with db_free().
db * db_read(FILE * f);

// db_read_filtered reads a database but filters out particular entries.
//
// The return-value semantics of db_read_filtered are the same as those for db_read.
db * db_read_filtered(FILE * f, db_filter filter);

// db_sort sorts a database chronologically.
void db_sort(db * database);

// db_free deallocates the result from db_read().
void db_free(db * d);

#endif

#ifndef __DB_H__
#define __DB_H__

#include "db_filter.h"

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

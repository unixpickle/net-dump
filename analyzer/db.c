#include "db.h"
#include <stdlib.h>
#include <string.h>

static int compare_entries_time(const void * ptr1, const void * ptr2);

db * db_read(FILE * f) {
  db_filter filter;
  filter.startTime = 0;
  filter.endTime = 0;
  filter.clientMAC[0] = 0;
  return db_read_filtered(f, filter);
}

db * db_read_filtered(FILE * f, db_filter filter) {
  int capacity = 16;
  db * database = (db *)malloc(sizeof(db));
  database->count = 0;
  database->entries = (db_entry *)malloc(capacity * sizeof(db_entry));

  while (1) {
    if (database->count == capacity) {
      capacity *= 2;
      database->entries = realloc(database->entries, sizeof(db_entry)*capacity);
    }
    db_entry * entry = database->entries + database->count;

    int res = db_entry_read(f, entry);
    if (res == 1) {
      db_free(database);
      return NULL;
    } else if (res == -1) {
      break;
    }

    if (!db_filter_matches(filter, entry)) {
      db_entry_free_fields(entry);
      continue;
    }

    ++database->count;
  }

  return database;
}

void db_sort(db * database) {
  qsort((void *)database->entries, database->count, sizeof(db_entry), compare_entries_time);
}

void db_free(db * database) {
  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry_free_fields(database->entries + i);
  }
  free(database->entries);
  free(database);
}

static int compare_entries_time(const void * e1, const void * e2) {
  db_entry * entry1 = (db_entry *)e1;
  db_entry * entry2 = (db_entry *)e2;
  if (entry1->timestamp < entry2->timestamp) {
    return -1;
  } else if (entry1->timestamp > entry2->timestamp) {
    return 1;
  } else {
    return 0;
  }
}

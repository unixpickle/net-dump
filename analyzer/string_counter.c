#include "string_counter.h"
#include <stdlib.h>
#include <string.h>

string_counter * string_counter_alloc() {
  string_counter * res = (string_counter *)malloc(sizeof(string_counter));
  res->entries = (string_counter_entry *)malloc(1);
  res->count = 0;
  return res;
}

// string_counter_add updates the number for the given string by adding `count` to it.
// This will create an entry for the string if one does not already exist.
void string_counter_add(string_counter * c, const char * str, unsigned long long count) {
  string_counter_entry * entry = NULL;

  int i;
  for (i = 0; i < c->count; ++i) {
    string_counter_entry * e = &c->entries[i];
    if (strcmp(e->str, str) == 0) {
      entry = e;
      break;
    }
  }

  if (entry == NULL) {
    size_t newSize = sizeof(string_counter_entry)*(c->count+1);
    c->entries = (string_counter_entry *)realloc(c->entries, newSize);

    entry = c->entries + c->count;
    ++c->count;

    bzero(entry, sizeof(*entry));
    entry->str = str;
  }

  entry->count += (unsigned long long)count;
}

// string_counter_sort sorts the entries in the counter in descending order.
void string_counter_sort(string_counter * c) {
  // TODO: use qsort() here.
  int keepLooping = 1;
  while (keepLooping) {
    keepLooping = 0;
    int i;
    for (i = 0; i < c->count-1; i++) {
      string_counter_entry e1 = c->entries[i];
      string_counter_entry e2 = c->entries[i+1];
      if (e2.count > e1.count) {
        keepLooping = 1;
        c->entries[i+1] = e1;
        c->entries[i] = e2;
      }
    }
  }
}

void string_counter_free(string_counter * c) {
  free(c->entries);
  free(c);
}

#include "string_counter.h"
#include <stdlib.h>
#include <string.h>

static int compare_entry_count(const void * e1, const void * e2);

string_counter * string_counter_alloc() {
  string_counter * res = (string_counter *)malloc(sizeof(string_counter));
  res->entries = (string_counter_entry *)malloc(1);
  res->count = 0;
  return res;
}

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
    entry->str = (char *)malloc(strlen(str) + 1);
    strcpy(entry->str, str);
  }

  entry->count += (unsigned long long)count;
}

void string_counter_sort(string_counter * c) {
  qsort(c->entries, c->count, sizeof(string_counter_entry), compare_entry_count);
}

void string_counter_free(string_counter * c) {
  int i;
  for (i = 0; i < c->count; ++i) {
    free(c->entries[i].str);
  }
  free(c->entries);
  free(c);
}

static int compare_entry_count(const void * e1, const void * e2) {
  string_counter_entry * entry1 = (string_counter_entry *)e1;
  string_counter_entry * entry2 = (string_counter_entry *)e2;
  if (entry1->count < entry2->count) {
    return 1;
  } else if (entry1->count > entry2->count) {
    return -1;
  } else {
    return 0;
  }
}

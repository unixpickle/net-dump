#include "clients.h"
#include "string_counter.h"
#include <stdio.h>
#include <string.h>

void hosts_command(int argc, const char ** argv, db * database) {
  string_counter * counter = string_counter_alloc();

  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    if (entry.request_host[0] == 0) {
      continue;
    }
    string_counter_add(counter, entry.request_host, 1);
  }

  string_counter_sort(counter);

  for (i = 0; i < counter->count; ++i) {
    string_counter_entry entry = counter->entries[i];

    printf("%s ", entry.str);

    int j;
    int len = strlen(entry.str);
    for (j = 0; j < 50-len; ++j) {
      printf(" ");
    }

    printf("%llu requests\n", entry.count);
  }

  string_counter_free(counter);
}

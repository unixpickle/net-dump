#include "clients.h"
#include "../string_counter.h"
#include <stdio.h>

void clients_command_help() {
  printf("No arguments for `clients`.\n");
}

void clients_command(int argc, const char ** argv, FILE * dbFile) {
  string_counter * counter = string_counter_alloc();

  while (1) {
    db_entry entry;
    int res = db_entry_read(dbFile, &entry);
    if (res == -1) {
      break;
    } else if (res == 1) {
      fprintf(stderr, "failed to read database.\n");
      string_counter_free(counter);
      return;
    }

    string_counter_add(counter, entry.client, (unsigned long long)entry.size);
    db_entry_free_fields(&entry);
  }

  string_counter_sort(counter);

  int i;
  for (i = 0; i < counter->count; ++i) {
    string_counter_entry entry = counter->entries[i];
    printf("%s - %llu bytes\n", entry.str, entry.count);
  }

  string_counter_free(counter);
}

#include "clients.h"
#include "string_counter.h"
#include <stdio.h>

void clients_command_help() {
  printf("No arguments for `clients`.\n");
}

void clients_command(int argc, const char ** argv, db * database) {
  string_counter * counter = string_counter_alloc();

  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    string_counter_add(counter, entry.client, (unsigned long long)entry.size);
  }

  string_counter_sort(counter);

  for (i = 0; i < counter->count; ++i) {
    string_counter_entry entry = counter->entries[i];
    printf("%s - %llu bytes\n", entry.str, entry.count);
  }

  string_counter_free(counter);
}

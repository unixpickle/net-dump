#include "clients.h"
#include <stdio.h>
#include <strings.h>

typedef struct {
  char *             address;
  unsigned long long total_data;
} client_entry;

static void sort_clients(client_entry * entries, int count);

void clients_command(int argc, const char ** argv, db * database) {
  client_entry * clients = malloc(1);
  int clientCount = 0;

  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    client_entry * listEntry = NULL;

    int j;
    for (j = 0; j < clientCount; ++j) {
      client_entry * e = &clients[j];
      if (strcmp(e->address, entry.client) == 0) {
        listEntry = e;
        break;
      }
    }

    if (listEntry == NULL) {
      clients = (client_entry *)realloc(clients, sizeof(client_entry)*(clientCount+1));
      listEntry = clients + clientCount;
      ++clientCount;
      bzero(listEntry, sizeof(*listEntry));
      listEntry->address = entry.client;
    }

    listEntry->total_data += (unsigned long long)entry.size;
  }

  sort_clients(clients, clientCount);

  for (i = 0; i < clientCount; ++i) {
    client_entry client = clients[i];
    printf("%s - %llu bytes\n", client.address, client.total_data);
  }

  free(clients);
}

static void sort_clients(client_entry * entries, int count) {
  // TODO: use qsort() here.
  int keepLooping = 1;
  while (keepLooping) {
    keepLooping = 0;
    int i;
    for (i = 0; i < count-1; i++) {
      client_entry e1 = entries[i];
      client_entry e2 = entries[i+1];
      if (e2.total_data > e1.total_data) {
        keepLooping = 1;
        entries[i+1] = e1;
        entries[i] = e2;
      }
    }
  }
}

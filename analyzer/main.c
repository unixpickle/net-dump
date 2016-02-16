#include <stdio.h>
#include <string.h>
#include "clients.h"
#include "db_entry.h"

static void print_usage();

int main(int argc, const char ** argv) {
  if (argc < 3) {
    print_usage(argv);
    return 1;
  }

  FILE * dbFile = fopen(argv[argc-1], "r");
  if (dbFile == NULL) {
    fprintf(stderr, "failed to open database file.\n");
    return 1;
  }
  db * database = db_read(dbFile);
  fclose(dbFile);
  if (database == NULL) {
    fprintf(stderr, "failed to read database file.\n");
    return 1;
  }

  const char * cmd = argv[1];
  if (strcmp(cmd, "clients") == 0) {
    clients_command(argc-3, argv+2, database);
  } else {
    fprintf(stderr, "unknown command: %s\n", cmd);
    return 1;
  }
}

static void print_usage(const char ** argv) {
  fprintf(stderr, "Usage: %s <command> [flags] <file.csv>\n"
    "Available commands are:\n"
    " clients       view all of the client MAC addresses\n", argv[0]);
}

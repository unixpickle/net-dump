#include <stdio.h>
#include <string.h>
#include "commands/clients.h"
#include "commands/histogram.h"
#include "commands/hosts.h"
#include "commands/usage.h"
#include "db.h"

static void print_usage();
static int print_help(const char * command);

int main(int argc, const char ** argv) {
  if (argc < 3) {
    print_usage(argv);
    return 1;
  }

  if (strcmp(argv[1], "help") == 0) {
    if (argc != 3) {
      fprintf(stderr, "`help` meta-command takes exactly one argument.\n");
      return 1;
    }
    return print_help(argv[2]);
  }

  FILE * dbFile = fopen(argv[argc-1], "r");
  if (dbFile == NULL) {
    fprintf(stderr, "failed to open database file.\n");
    return 1;
  }

  const char * names[] = {"clients", "hosts", "usage", "histogram"};
  void (* commands[])(int, const char **, FILE *) = {
    clients_command,
    hosts_command,
    usage_command,
    histogram_command
  };

  const char * cmd = argv[1];
  int i;
  for (i = 0; i < sizeof(names)/sizeof(*names); ++i) {
    if (strcmp(names[i], cmd) == 0) {
      commands[i](argc-3, argv+2, dbFile);
      fclose(dbFile);
      return 0;
    }
  }

  fclose(dbFile);
  fprintf(stderr, "unknown command: %s\n", cmd);
  return 1;
}

static void print_usage(const char ** argv) {
  fprintf(stderr, "Usage: %s <command> [flags] <file.csv>\n"
    "  or   %s help <command>\n"
    "Available commands are:\n"
    " clients       list client MAC addresses\n"
    " hosts         list HTTP/1.1 Host header values\n"
    " usage         graph data usage over time\n"
    " histogram     print histograms for average data usage\n", argv[0], argv[0]);
}

static int print_help(const char * command) {
  const char * names[] = {"clients", "usage", "hosts", "histogram"};
  void (* helpers[])() = {
    clients_command_help,
    usage_command_help,
    hosts_command_help,
    histogram_command_help
  };
  int i;
  for (i = 0; i < sizeof(helpers)/sizeof(*helpers); ++i) {
    if (strcmp(names[i], command) == 0) {
      helpers[i]();
      return 0;
    }
  }
  fprintf(stderr, "Unknown command: %s\n", command);
  return 1;
}

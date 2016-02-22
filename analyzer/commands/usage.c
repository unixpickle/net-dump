#include "usage.h"
#include "../flags.h"
#include "../graph.h"
#include <stdlib.h>
#include <string.h>

void usage_command_help() {
  printf("Available flags:\n\n"
    " -s <start time>  starting epoch time.\n"
    " -e <end time>    ending epoch time.\n"
    " -d <filter MAC>  focus on a specific MAC.\n"
    "\n");
}

void usage_command(int argc, const char ** argv, FILE * dbFile) {
  cmd_flags * flags = cmd_flags_parse("-s time -e time -d string", argc, argv);
  if (flags == NULL) {
    return;
  }
  db_filter filter = db_filter_from_flags(flags);
  cmd_flags_free(flags);

  db * database = db_read_filtered(dbFile, filter);
  if (database == NULL) {
    fprintf(stderr, "failed to read database.\n");
    return;
  }

  if (database->count == 0) {
    db_free(database);
    printf("nothing to graph.\n");
    return;
  }

  db_sort(database);

  int bufferSize = 16;
  int valueCount = 0;
  double * values = (double *)malloc(sizeof(double) * bufferSize);

  unsigned long long currentMinute = database->entries[0].timestamp;
  double currentValue = 0;
  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    while (entry.timestamp >= currentMinute+60) {
      if (valueCount == bufferSize) {
        bufferSize *= 2;
        values = realloc(values, bufferSize * sizeof(double));
      }
      values[valueCount++] = currentValue;
      currentValue = 0;
      currentMinute += 60;
    }
    currentValue += (double)entry.size;
  }

  if (valueCount == bufferSize) {
    bufferSize += 1;
    values = realloc(values, bufferSize * sizeof(double));
  }
  values[valueCount++] = currentValue;

  printf("Data usage (spanning %lf hours):\n", (double)valueCount / 60);
  print_graph(values, valueCount);

  free(values);
  db_free(database);
}

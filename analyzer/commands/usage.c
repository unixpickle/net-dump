#include "usage.h"
#include "../flags.h"
#include "../graph.h"
#include <string.h>

void usage_command_help() {
  printf("Available flags:\n\n"
    " -s <start time>  starting epoch time.\n"
    " -e <end time>    ending epoch time.\n"
    " -d <filter MAC>  focus on a specific MAC.\n"
    "\n");
}

void usage_command(int argc, const char ** argv, db * database) {
  if (database->count == 0) {
    printf("Nothing to graph.\n");
    return;
  }

  cmd_flags * flags = cmd_flags_parse("-s time -e time -d string", argc, argv);
  if (flags == NULL) {
    return;
  }

  db_sort(database);

  unsigned long long startTime = cmd_flags_get_time(flags, "-s", database->entries[0].timestamp);
  unsigned long long endTime = cmd_flags_get_time(flags, "-e",
    database->entries[database->count-1].timestamp);
  const char * filterMAC = cmd_flags_get_string(flags, "-d", NULL);
  cmd_flags_free(flags);

  int bufferSize = 16;
  int valueCount = 0;
  double * values = (double *)malloc(sizeof(double) * bufferSize);

  unsigned long long currentMinute = startTime;
  double currentValue = 0;
  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    if (entry.timestamp < startTime) {
      continue;
    } else if (entry.timestamp > endTime) {
      continue;
    } else if (filterMAC != NULL && strcmp(entry.client, filterMAC) != 0) {
      continue;
    }
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
}

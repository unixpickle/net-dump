#include "usage.h"
#include "../graph.h"

void usage_command_help() {
  printf("No arguments for `usage`.\n");
}

void usage_command(int argc, const char ** argv, db * database) {
  if (database->count == 0) {
    printf("Nothing to graph.\n");
    return;
  }

  db_sort(database);

  int bufferSize = 16;
  int valueCount = 0;
  double * values = (double *)malloc(sizeof(double) * bufferSize);

  unsigned long long firstTimestamp = database->entries[0].timestamp;
  unsigned long long currentMinute = firstTimestamp;
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
}

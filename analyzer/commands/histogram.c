#include "histogram.h"
#include "../db.h"
#include "../graph.h"
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#define DAY_LENGTH 24
#define WEEK_LENGTH 7

static double * weekly_histogram(FILE * dbFile, db_filter filter);
static double * daily_histogram(FILE * dbFile, db_filter filter);
static double * histogram(FILE * dbFile, db_filter filter, int valueCount,
                          int (* category)(time_t ts));

static int timestamp_hour(time_t ts);
static int timestamp_day_of_week(time_t ts);

void histogram_command_help() {
  printf("Available flags:\n\n"
    " -w               show a weekly histogram rather than a daily one.\n"
    " -m               a bitmask indicating which days of the week to\n"
    "                  include. Default: 1111111."
    " -s <start time>  starting epoch time.\n"
    " -e <end time>    ending epoch time.\n"
    " -d <filter MAC>  focus on a specific MAC.\n"
    "\n");
}

void histogram_command(int argc, const char ** argv, FILE * dbFile) {
  cmd_flags * flags = cmd_flags_parse("-w bool -m string -s time -e time -d string", argc, argv);
  if (flags == NULL) {
    return;
  }
  int weekly = cmd_flags_get_int(flags, "-w", 0);
  db_filter filter = db_filter_from_flags(flags);
  cmd_flags_free(flags);

  double * histogram;
  int histogramCount;
  if (weekly) {
    histogram = weekly_histogram(dbFile, filter);
    histogramCount = WEEK_LENGTH;
  } else {
    histogram = daily_histogram(dbFile, filter);
    histogramCount = DAY_LENGTH;
  }

  if (histogram == NULL) {
    fprintf(stderr, "failed to read database.\n");
    return;
  }

  int graphWidth = histogramCount * (DEFAULT_GRAPH_WIDTH / histogramCount);
  if (graphWidth == 0) {
    graphWidth = histogramCount;
  }

  print_graph_at_size(histogram, histogramCount, graphWidth, DEFAULT_GRAPH_HEIGHT);
  free(histogram);
}

static double * weekly_histogram(FILE * dbFile, db_filter filter) {
  return histogram(dbFile, filter, WEEK_LENGTH, timestamp_day_of_week);
}

static double * daily_histogram(FILE * dbFile, db_filter filter) {
  return histogram(dbFile, filter, DAY_LENGTH, timestamp_hour);
}

static double * histogram(FILE * dbFile, db_filter filter, int valueCount,
                          int (* category)(time_t ts)) {
  double * res = (double *)malloc(valueCount * sizeof(double));
  bzero(res, valueCount*sizeof(double));

  while (1) {
    db_entry entry;
    int r = db_entry_read(dbFile, &entry);
    if (r == -1) {
      break;
    } else if (r == 1) {
      free(res);
      return NULL;
    }

    if (!db_filter_matches(filter, &entry)) {
      db_entry_free_fields(&entry);
      continue;
    }

    res[category((time_t)entry.timestamp)] += 1;

    db_entry_free_fields(&entry);
  }

  return res;
}

static int timestamp_hour(time_t ts) {
  struct tm t;
  localtime_r(&ts, &t);
  return t.tm_hour;
}

static int timestamp_day_of_week(time_t ts) {
  struct tm t;
  localtime_r(&ts, &t);
  return t.tm_wday;
}

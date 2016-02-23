#include "clients.h"
#include "../flags.h"
#include "../string_counter.h"
#include <stdio.h>
#include <string.h>

static char * find_cookie_domain(char * headers);

void hosts_command_help() {
  printf("Available flags:\n\n"
    " -r               only extract hosts from requests, not responses.\n"
    " -s <start time>  starting epoch time.\n"
    " -e <end time>    ending epoch time.\n"
    " -d <filter MAC>  focus on a specific MAC.\n"
    "\n");
}

void hosts_command(int argc, const char ** argv, FILE * dbFile) {
  cmd_flags * flags = cmd_flags_parse("-r bool -s time -e time -d string", argc, argv);
  if (flags == NULL) {
    return;
  }
  int useCookies = 1 - cmd_flags_get_int(flags, "-r", 0);
  db_filter filter = db_filter_from_flags(flags);
  cmd_flags_free(flags);

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

    if (!db_filter_matches(filter, &entry)) {
      db_entry_free_fields(&entry);
      continue;
    }
    if (entry.request_host[0] != 0) {
      string_counter_add(counter, entry.request_host, 1);
    }
    if (useCookies) {
      char * cookieDomain = find_cookie_domain(entry.response_headers);
      if (cookieDomain != NULL) {
        string_counter_add(counter, cookieDomain, 1);
      }
    }
    db_entry_free_fields(&entry);
  }

  string_counter_sort(counter);

  int i;
  for (i = 0; i < counter->count; ++i) {
    string_counter_entry entry = counter->entries[i];

    printf("%s ", entry.str);

    int j;
    int len = strlen(entry.str);
    for (j = 0; j < 50-len; ++j) {
      printf(" ");
    }

    printf("%llu occurrences\n", entry.count);
  }

  string_counter_free(counter);
}

static char * find_cookie_domain(char * headers) {
  // TODO: avoid false positives by specifically making sure we are in a "Cookie" header.

  ssize_t len = strlen(headers);
  ssize_t i;
  for (i = 0; i < len-7; ++i) {
    if (memcmp(headers+i, "Domain=", 7) == 0) {
      ssize_t start = i+7;
      for (i = start; i < len; ++i) {
        char ch = headers[i];
        if (ch == ';' || ch == '\n') {
          break;
        }
      }
      headers[i] = 0;
      return headers + start;
    }
  }
  return NULL;
}

#include "clients.h"
#include "string_counter.h"
#include <stdio.h>
#include <string.h>

static char * find_cookie_domain(char * headers);

void hosts_command_help() {
  printf("Available flags:\n\n"
    " -r            only extract hosts from requests, not responses.\n\n");
}

void hosts_command(int argc, const char ** argv, db * database) {
  int useCookies = 1;
  if (argc > 0) {
    if (argc != 1) {
      fprintf(stderr, "invalid arguments.\n");
      return;
    }
    if (strcmp(argv[0], "-r") != 0) {
      fprintf(stderr, "invalid arguments.\n");
      return;
    }
    useCookies = 0;
  }

  string_counter * counter = string_counter_alloc();

  int i;
  for (i = 0; i < database->count; ++i) {
    db_entry entry = database->entries[i];
    if (entry.request_host[0] != 0) {
      string_counter_add(counter, entry.request_host, 1);
    }
    if (useCookies) {
      char * cookieDomain = find_cookie_domain(entry.response_headers);
      if (cookieDomain != NULL) {
        string_counter_add(counter, cookieDomain, 1);
      }
    }
  }

  string_counter_sort(counter);

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
        if (ch == ';' || ch == '\\') {
          break;
        }
      }
      headers[i] = 0;
      return headers + start;
    }
  }
  return NULL;
}

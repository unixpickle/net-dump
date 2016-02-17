#include "db_entry.h"
#include <errno.h>
#include <string.h>

static int read_line(FILE * f, char ** out);
static int count_commas(const char * str);
static int null_out_commas(char * str);

db * db_read(FILE * f) {
  int capacity = 16;
  db * database = (db *)malloc(sizeof(db));
  database->count = 0;
  database->entries = (db_entry *)malloc(capacity * sizeof(db_entry));

  while (1) {
    char * line;
    int res = read_line(f, &line);
    if (res == 1) {
      db_free(database);
      return NULL;
    }

    if (strlen(line) == 0) {
      free(line);
      if (res == EOF) {
        break;
      } else {
        continue;
      }
    }

    int commaCount = count_commas(line);
    if (commaCount != 9) {
      free(line);
      db_free(database);
      return NULL;
    }

    if (null_out_commas(line) < 0) {
      free(line);
      db_free(database);
      return NULL;
    }

    if (database->count == capacity) {
      capacity *= 2;
      database->entries = realloc(database->entries, sizeof(db_entry)*capacity);
    }
    db_entry * entry = database->entries + database->count;
    ++database->count;

    // After we set entry->type, calling db_free(database) will free the current line, so we no
    // longer have to worry about freeing it ourselves upon error.
    entry->type = line;
    line += strlen(line) + 1;

    entry->timestamp = strtoull(line, NULL, 10);
    errno = 0;
    if (errno || entry->timestamp == 0) {
      db_free(database);
      return NULL;
    }
    line += strlen(line) + 1;

    entry->rssi = atoi(line);
    line += strlen(line) + 1;

    errno = 0;
    entry->size = (size_t)strtoull(line, NULL, 10);
    if (errno || entry->size == 0) {
      db_free(database);
      return NULL;
    }

    char ** fields[] = {&entry->client, &entry->access_point, &entry->request_path,
      &entry->request_host, &entry->request_user_agent, &entry->response_headers, NULL};
    char *** fieldsPtr = fields;

    for (; *fieldsPtr; fieldsPtr += 1) {
      line += strlen(line) + 1;
      (*(*fieldsPtr)) = line;
    }

    if (res == EOF) {
      break;
    }
  }

  return database;
}

void db_free(db * database) {
  int i;
  for (i = 0; i < database->count; ++i) {
    // All of the strings in the db_entry are part of the same malloc()'d buffer.
    db_entry entry = database->entries[i];
    free(entry.type);
  }
  free(database->entries);
  free(database);
}

static int read_line(FILE * f, char ** out) {
  int len = 0;
  int capacity = 80;
  char * res = malloc(capacity);
  res[0] = 0;

  while (1) {
    if (fgets(res+len, capacity-len, f) == NULL) {
      if (feof(f)) {
        (*out) = res;
        return EOF;
      } else {
        free(res);
        return 1;
      }
    }

    if (feof(f)) {
      (*out) = res;
      return EOF;
    }

    len += strlen(res+len);
    if (res[len-1] == '\n') {
      res[--len] = 0;
      (*out) = res;
      return 0;
    }

    capacity += 20;
    res = realloc(res, capacity);
  }
}

static int count_commas(const char * str) {
  int count = 0;
  for (; *str; ++str) {
    if ((*str) == '\\') {
      ++str;
      if (!(*str)) {
        return -1;
      }
    } else if ((*str) == ',') {
      ++count;
    }
  }
  return count;
}

static int null_out_commas(char * str) {
  for (; *str; ++str) {
    if ((*str) == '\\') {
      ++str;
      if (!(*str)) {
        return -1;
      }
    } else if ((*str) == ',') {
      (*str) = 0;
    }
  }
  return 0;
}

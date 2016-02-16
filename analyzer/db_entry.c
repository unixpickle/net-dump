#include "db_entry.h"
#include <errno.h>
#include <string.h>

static int read_line(FILE * f, char ** out);
static int count_commas(const char * str);
static int null_out_commas(char * str);

db * db_read(FILE * f) {
  db * database = (db *)malloc(sizeof(db));
  database->count = 0;
  database->entries = (db_entry *)malloc(1);

  while (1) {
    char * line;
    int res = read_line(f, &line);
    if (res == 1) {
      db_free(database);
      return NULL;
    }

    if (strlen(line) == 0) {
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

    database->entries = realloc(database->entries, sizeof(db_entry)*(database->count+1));
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
  char * res = malloc(1);
  res[0] = 0;

  while (1) {
    int ch = fgetc(f);
    if (ch == EOF) {
      if (ferror(f)) {
        free(res);
        return 1;
      } else {
        (*out) = res;
        return EOF;
      }
    }

    if (ch == '\n') {
      (*out) = res;
      return 0;
    }

    res = realloc(res, len+2);
    res[len++] = (char)ch;
    res[len] = 0;
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

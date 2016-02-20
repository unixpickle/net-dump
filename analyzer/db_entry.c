#include "db_entry.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int read_csv_row(FILE * f, char ** out);
static int compare_entries_time(const void * ptr1, const void * ptr2);

#define CSV_PARSER_STARTING_CAPACITY 80

typedef enum {
  PARSER_STATE_FIELD_FIRST = 0,
  PARSER_STATE_QUOTED_FIELD,
  PARSER_STATE_UNQUOTED_FIELD,
  PARSER_STATE_SEEN_QUOTE,
  PARSER_STATE_ERROR,
  PARSER_STATE_DONE
} parser_state;

typedef struct {
  char * joinedFields;
  int fieldCount;

  int _capacity;
  int _len;
  parser_state _state;
} csv_row_parser;

static csv_row_parser * csv_row_parser_alloc();
static void csv_row_parser_next(csv_row_parser * p, int ch);
static int csv_row_parser_done(csv_row_parser * p);
static int csv_row_parser_error(csv_row_parser * p);
static void csv_row_parser_free_struct(csv_row_parser * p);
static void csv_row_parser_free_all(csv_row_parser * p);

db * db_read(FILE * f) {
  int capacity = 16;
  db * database = (db *)malloc(sizeof(db));
  database->count = 0;
  database->entries = (db_entry *)malloc(capacity * sizeof(db_entry));

  while (1) {
    if (feof(f)) {
      break;
    }

    char * line;
    int fieldCount = read_csv_row(f, &line);
    if (fieldCount == 0) {
      continue;
    } else if (fieldCount < 0) {
      db_free(database);
      return NULL;
    }

    if (fieldCount != 10) {
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

    errno = 0;
    entry->timestamp = strtoull(line, NULL, 10);
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
  }

  return database;
}

void db_sort(db * database) {
  qsort((void *)database->entries, database->count, sizeof(db_entry), compare_entries_time);
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

static int read_csv_row(FILE * f, char ** out) {
  csv_row_parser * p = csv_row_parser_alloc();

  flockfile(f);
  while (!csv_row_parser_done(p) && !csv_row_parser_error(p)) {
    int nextChar = getc_unlocked(f);

    if (nextChar == EOF) {
      funlockfile(f);
      if (ferror(f)) {
        csv_row_parser_free_all(p);
        return -1;
      }
      flockfile(f);
    }

    csv_row_parser_next(p, nextChar);
  }
  funlockfile(f);

  if (csv_row_parser_error(p)) {
    csv_row_parser_free_all(p);
    return -1;
  } else if (p->fieldCount == 1 && !p->joinedFields[0]) {
    csv_row_parser_free_all(p);
    return 0;
  }

  (*out) = p->joinedFields;
  int count = p->fieldCount;
  csv_row_parser_free_struct(p);
  return count;
}

static int compare_entries_time(const void * e1, const void * e2) {
  db_entry * entry1 = (db_entry *)e1;
  db_entry * entry2 = (db_entry *)e2;
  if (entry1->timestamp < entry2->timestamp) {
    return -1;
  } else if (entry1->timestamp > entry2->timestamp) {
    return 1;
  } else {
    return 0;
  }
}

static csv_row_parser * csv_row_parser_alloc() {
  csv_row_parser * res = (csv_row_parser *)malloc(sizeof(csv_row_parser));
  bzero(res, sizeof(*res));
  res->_capacity = CSV_PARSER_STARTING_CAPACITY;
  res->joinedFields = (char *)malloc(res->_capacity);
  res->fieldCount = 1;
  return res;
}

static void csv_row_parser_next(csv_row_parser * p, int ch) {
  int appendChar = -1;

  switch (p->_state) {
  case PARSER_STATE_FIELD_FIRST:
    switch (ch) {
    case -1:
    case '\n':
      appendChar = 0;
      p->_state = PARSER_STATE_DONE;
      break;
    case '"':
      p->_state = PARSER_STATE_QUOTED_FIELD;
      break;
    case ',':
      ++p->fieldCount;
      appendChar = 0;
      break;
    default:
      p->_state = PARSER_STATE_UNQUOTED_FIELD;
      appendChar = ch;
    }
    break;
  case PARSER_STATE_QUOTED_FIELD:
    switch (ch) {
    case '"':
      p->_state = PARSER_STATE_SEEN_QUOTE;
      break;
    default:
      appendChar = ch;
    }
    break;
  case PARSER_STATE_UNQUOTED_FIELD:
    switch (ch) {
    case '"':
      p->_state = PARSER_STATE_ERROR;
      break;
    case ',':
      appendChar = 0;
      ++p->fieldCount;
      p->_state = PARSER_STATE_FIELD_FIRST;
      break;
    case '\n':
      appendChar = 0;
      p->_state = PARSER_STATE_DONE;
      break;
    default:
      appendChar = ch;
    }
    break;
  case PARSER_STATE_SEEN_QUOTE:
    switch (ch) {
    case -1:
    case '\n':
      appendChar = 0;
      p->_state = PARSER_STATE_DONE;
      break;
    case ',':
      appendChar = 0;
      p->_state = PARSER_STATE_FIELD_FIRST;
      ++p->fieldCount;
      break;
    case '"':
      appendChar = '"';
      p->_state = PARSER_STATE_QUOTED_FIELD;
      break;
    default:
      p->_state = PARSER_STATE_ERROR;
    }
    break;
  default:
    return;
  }

  if (appendChar < 0) {
    return;
  }

  if (p->_capacity == p->_len) {
    p->_capacity *= 2;
    p->joinedFields = realloc(p->joinedFields, p->_capacity);
  }
  p->joinedFields[p->_len++] = (char)appendChar;
}

static int csv_row_parser_done(csv_row_parser * p) {
  return p->_state == PARSER_STATE_DONE;
}

static int csv_row_parser_error(csv_row_parser * p) {
  return p->_state == PARSER_STATE_ERROR;
}

static void csv_row_parser_free_struct(csv_row_parser * p) {
  free(p);
}

static void csv_row_parser_free_all(csv_row_parser * p) {
  free(p->joinedFields);
  free(p);
}

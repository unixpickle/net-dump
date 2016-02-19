#include "flags.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t next_field(const char * str, const char ** fieldStart);
static const char * flag_type(const char * options, const char * flagName);

cmd_flags * cmd_flags_parse(const char * options, int argc, const char ** argv) {
  cmd_flags * res = (cmd_flags *)malloc(sizeof(cmd_flags));
  res->flags = (cmd_flag *)malloc(1);
  res->count = 0;

  int i;
  for (i = 0; i < argc; ++i) {
    const char * flagName = argv[i];
    const char * type = flag_type(options, flagName);
    if (type == NULL) {
      fprintf(stderr, "unknown flag: %s\n", flagName);
      cmd_flags_free(res);
      return NULL;
    }

    if (strcmp(type, "bool") != 0) {
      ++i;
      if (i == argc) {
        fprintf(stderr, "missing parameter for %s flag.\n", flagName);
        cmd_flags_free(res);
        return NULL;
      }
    }

    res->flags = (cmd_flag *)realloc(res->flags, sizeof(cmd_flag)*(res->count+1));
    cmd_flag * flag = res->flags + res->count;
    res->count++;

    flag->name = flagName;

    if (strcmp(type, "bool") == 0) {
      flag->type = CMD_FLAG_TYPE_INT;
      flag->value.integer = 1;
    } else if (strcmp(type, "int") == 0) {
      flag->type = CMD_FLAG_TYPE_INT;
      flag->value.integer = atoi(argv[i]);
      // TODO: validate the integer value here.
    } else if (strcmp(type, "string") == 0) {
      flag->type = CMD_FLAG_TYPE_STRING;
      flag->value.string = argv[i];
    } else if (strcmp(type, "time") == 0) {
      flag->type = CMD_FLAG_TYPE_TIME;
      errno = 0;
      flag->value.timestamp = strtoull(argv[i], NULL, 10);
      if (errno || flag->value.timestamp == 0) {
        fprintf(stderr, "invalid value for flag %s: %s\n", flagName, argv[i]);
        cmd_flags_free(res);
        return NULL;
      }
    }
  }

  return res;
}

#define CMD_FLAGS_GETTER(x) int i;\
  for (i = 0; i < flags->count; ++i) {\
    if (strcmp(flags->flags[i].name, name) == 0) {\
      return flags->flags[i].value.x;\
    }\
  }\
  return defaultValue;\

int cmd_flags_get_int(cmd_flags * flags, const char * name, int defaultValue) {
  CMD_FLAGS_GETTER(integer);
}

const char * cmd_flags_get_string(cmd_flags * flags, const char * name, const char * defaultValue) {
  CMD_FLAGS_GETTER(string);
}

unsigned long long cmd_flags_get_time(cmd_flags * flags, const char * name,
                                      unsigned long long defaultValue) {
  CMD_FLAGS_GETTER(timestamp);
}

void cmd_flags_free(cmd_flags * flags) {
  free(flags->flags);
  free(flags);
}

static size_t next_field(const char * str, const char ** fieldStart) {
  while (1) {
    if ((*str) == 0) {
      return 0;
    } else if ((*str) != ' ') {
      break;
    }
    ++str;
  }

  (*fieldStart) = str;

  size_t len;
  for (len = 0; (*str) != ' ' && (*str) != 0; ++str, ++len) {
  }
  return len;
}

static const char * flag_type(const char * options, const char * flagName) {
  size_t flagLen = strlen(flagName);
  while (1) {
    size_t len = next_field(options, &options);
    if (len == 0) {
      break;
    }
    if (len != flagLen || memcmp(options, flagName, len) != 0) {
      options += len;
      len = next_field(options, &options);
      if (len == 0) {
        break;
      }
      options += len;
      continue;
    }
    options += len;
    len = next_field(options, &options);
    if (len == 0) {
      break;
    }
    const char * types[] = {"string", "time", "int", "bool"};
    int j;
    for (j = 0; j < sizeof(types)/sizeof(*types); ++j) {
      if (len == strlen(types[j])) {
        if (memcmp(options, types[j], len) == 0) {
          return types[j];
        }
      }
    }
    return NULL;
  }
  return NULL;
}

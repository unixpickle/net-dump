#ifndef __FLAGS_H__
#define __FLAGS_H__

typedef enum {
  CMD_FLAG_TYPE_INT = 0,
  CMD_FLAG_TYPE_STRING = 1,
  CMD_FLAG_TYPE_TIME = 2,
} cmd_flag_type;

typedef struct {
  cmd_flag_type type;
  const char *  name;

  union {
    int                integer;
    const char *       string;
    unsigned long long timestamp;
  } value;
} cmd_flag;

typedef struct {
  int        count;
  cmd_flag * flags;
} cmd_flags;

// cmd_flags_parse parses flags from an arguments list.
//
// The options string specifies the possible flags and their associated types in a concise way.
// The string is of the form "-flag1 type1 -flag2 type2 ..."; it is a space separated list where
// every even-indexed entry is a flag name and every odd-indexed entry is the type for the previous
// flag.
//
// The available types are "string", "int", "time", and "bool".
// Flags of the "bool" type take no argument, and are expressed as integer fields which are 1 when
// the flag was supplied.
//
// This returns NULL if the flags could not be parsed.
// In this case, this will print out an error before returning.
//
// If this returns a non-NULL result, said result should be freed with cmd_flags_free.
cmd_flags * cmd_flags_parse(const char * options, int argc, const char ** argv);

int cmd_flags_get_int(cmd_flags * flags, const char * name, int defaultValue);

const char * cmd_flags_get_string(cmd_flags * flags, const char * name, const char * defaultValue);

unsigned long long cmd_flags_get_time(cmd_flags * flags, const char * name,
                                      unsigned long long defaultValue);

void cmd_flags_free(cmd_flags * flags);

#endif

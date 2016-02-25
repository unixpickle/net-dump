#include "db_filter.h"
#include <string.h>

db_filter db_filter_from_flags(cmd_flags * flags) {
  db_filter res;

  res.startTime = cmd_flags_get_time(flags, "-s", 0);
  res.endTime = cmd_flags_get_time(flags, "-e", 0);

  const char * macStr = cmd_flags_get_string(flags, "-d", "");
  if (strlen(macStr) == sizeof(res.clientMAC)-1) {
    memcpy(res.clientMAC, macStr, sizeof(res.clientMAC));
  } else {
    res.clientMAC[0] = 0;
  }

  return res;
}

int db_filter_matches(db_filter f, db_entry * e) {
  if (f.clientMAC[0]) {
    if (strcmp(f.clientMAC, e->client) != 0) {
      return 0;
    }
  }
  if (f.startTime) {
    if (e->timestamp < f.startTime) {
      return 0;
    }
  }
  if (f.endTime) {
    if (e->timestamp > f.endTime) {
      return 0;
    }
  }
  return 1;
}

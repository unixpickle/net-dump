#include "db_filter.h"
#include <string.h>
#include <time.h>

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

  const char * dayMask = cmd_flags_get_string(flags, "-m", "1111111");
  if (strlen(dayMask) != 7) {
    dayMask = "1111111";
  }

  res.dayMask = 0;
  int i;
  for (i = 0; i < strlen(dayMask); ++i) {
    if (dayMask[i] == '1') {
      res.dayMask |= (1 << i);
    }
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
  if (f.dayMask != 0x7f) {
    struct tm t;
    time_t timestamp = (time_t)e->timestamp;
    gmtime_r(&timestamp, &t);
    if (!(f.dayMask & (1 << t.tm_wday))) {
      return 0;
    }
  }
  return 1;
}

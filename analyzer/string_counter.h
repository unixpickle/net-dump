typedef struct {
  char * str;
  unsigned long long count;
} string_counter_entry;

// A string_counter maps strings to numbers, with exactly one number per string.
typedef struct {
  int                    count;
  string_counter_entry * entries;
} string_counter;

string_counter * string_counter_alloc();

// string_counter_add updates the number for the given string by adding `count` to it.
// This will create an entry for the string if one does not already exist.
void string_counter_add(string_counter * c, const char * str, unsigned long long count);

// string_counter_sort sorts the entries in the counter in descending order.
void string_counter_sort(string_counter * c);

void string_counter_free(string_counter * c);

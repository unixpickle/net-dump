#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t dieMutex = PTHREAD_MUTEX_INITIALIZER;

void die(const char * message) {
  pthread_mutex_lock(&dieMutex);
  fprintf(stderr, "%s\n", message);
  exit(1);
}

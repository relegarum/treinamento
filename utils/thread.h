#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include <stdint.h>

#include "connection_item.h"

typedef struct thread_struct
{
  Connection *item;
  int32_t     id;
  int32_t     ret;
  pthread_t   p_thread;
} thread;

void init_thread(thread *this, int32_t id);
void start_thread(thread *this);
void clean_thread(thread *this);

#endif // THREAD_H

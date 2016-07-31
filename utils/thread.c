#include "thread.h"



void init_thread(thread *this, int32_t id)
{
  this->id = id;
  this->ret = 0;
}

void start_thread(thread *this)
{

}

void clean_thread(thread *this)
{
  pthread_exit(&(this->ret));
}

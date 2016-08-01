#include "thread.h"
#include <unistd.h>



void init_thread(thread *this, request_manager *manager, int32_t id)
{
  this->manager = manager;
  this->id = id;
  this->ret = 0;
  printf("Manager init: %d size:%d\n", id, this->manager->size);
}

void start_thread(thread *this)
{
  pthread_create(&(this->pthread), NULL, &do_thread, (void *)(this->manager));
}

/*void clean_thread(thread *this)
{
  printf("cleaning %d", this->id);
  pthread_exit(&(this->ret));
}*/

void *do_thread(void *this)
{
  request_manager *manager = (request_manager *)(this);
  /*int32_t id = pair->id;*/

  //printf("Start thread %d\n", pair->id);
  //printf("Manager %d\n", manager->size);

  while (1)
  {
    while(manager == NULL)
    {
      printf("manager is null");
    }
    pthread_mutex_lock(&(manager->mutex));
    while (manager->size == 0)
    {
      pthread_cond_wait(&(manager->conditional_variable), &(manager->mutex));
      if (manager->exit)
      {
        pthread_exit(NULL);
      }
    }

    request_list_node *item = manager->head;
    remove_request_in_list(manager, item);
    pthread_mutex_unlock(&(manager->mutex));

    handle_request_item(item);
  }
}


void handle_request_item(request_list_node *item)
{
  printf(" Data Size: %d\n", item->data_size);
  printf(" Operation: %d\n", item->operation);
  printf(" Id: %d\n",      item->id);
}

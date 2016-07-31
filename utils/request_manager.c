#include "request_manager.h"
#include <unistd.h>
#include <stdlib.h>

request_manager create_request_manager()
{
  request_manager manager;
  init_request_list(&manager);
  return manager;
}


void init_request_list(request_manager *manager)
{
  manager->head = NULL;
  manager->tail = NULL;
  manager->size = 0;
}


void add_request_in_list(request_manager *manager, request_list_node *new_item)
{
  pthread_rwlock_wrlock(&(manager->lock));
  ++(manager->size);
  if (manager->head == NULL)
  {
    manager->head = new_item;
    manager->tail = new_item;
  }
  else
  {
    new_item->previous_ptr = manager->tail;
    manager->tail->next_ptr = new_item;
    manager->tail = new_item;
  }
  pthread_rwlock_unlock(&(manager->lock));
}


void remove_request_in_list(request_manager *manager, request_list_node *item)
{
  if (item == NULL)
  {
    printf("\nNull connection cannot be removed\n");
    return;
  }

  pthread_rwlock_wrlock(&(manager->lock));
  --(manager->size);
  if ((item->previous_ptr != NULL) &&
      (item->next_ptr != NULL))
  {
    item->previous_ptr->next_ptr = item->next_ptr;
    item->next_ptr->previous_ptr = item->previous_ptr;
  }
  else if ((item->previous_ptr == NULL) && /*Only Header case*/
           (item->next_ptr == NULL))
  {
    destroy_node(item);
    free(item);
    /*item = NULL; Check this*/
    manager->head = NULL;
    manager->tail = NULL;
    return;
  }
  else if (item->previous_ptr == NULL) /*Header case*/
  {
    item->next_ptr->previous_ptr = NULL;
    manager->head = item->next_ptr;
  }
  else if (item->next_ptr == NULL) /*Tail case*/
  {
    item->previous_ptr->next_ptr = NULL;
    manager->tail = item->previous_ptr;
  }

  destroy_node(item);
  free(item);
  /*item = NULL; Check this*/
  pthread_rwlock_unlock(&(manager->lock));
}


void free_request_list(request_manager *manager)
{
  while(manager->head != NULL)
  {
    remove_request_in_list(manager, (manager->head));
  }

  manager->head = NULL;
  manager->tail = NULL;
}

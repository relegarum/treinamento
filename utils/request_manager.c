/* \file request_manager.c
 *
 * \brief Contem a implementacao das funcoes de manipulacao da lista de
 * requisicoes a serem tratadas pelas threads. Como ela e o dado a ser
 * compartilhado entre as thread, contem tambem os elementos de controle de
 * concorrencia.
 *
 * "$Id: $"
*/
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
  pthread_mutex_init(&(manager->mutex), NULL);
  pthread_cond_init(&(manager->conditional_variable), NULL);
  manager->number_of_threads = 0;
  manager->exit = 0;
}

void add_request_in_list(request_manager *manager, request_list_node *new_item)
{
  pthread_mutex_lock(&(manager->mutex));
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
  ++(manager->size);

  pthread_cond_signal(&(manager->conditional_variable));
  pthread_mutex_unlock(&(manager->mutex));
}

void remove_request_in_list(request_manager *manager, request_list_node *item)
{
  if (item == NULL)
  {
    printf("\nNull connection cannot be removed\n");
    return;
  }

  if (manager->head == NULL ||
      manager->size <= 0)
  {
    printf("\nManager in invalid state\n");
    return;
  }

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
}

void free_request_list(request_manager *manager)
{
  pthread_mutex_lock(&(manager->mutex));
  while(manager->head != NULL)
  {
    request_list_node *head = (manager->head);
    remove_request_in_list(manager, head);
    destroy_node(head);
  }

  manager->head = NULL;
  manager->tail = NULL;
  manager->exit = 1;

  pthread_cond_broadcast(&(manager->conditional_variable));
  pthread_mutex_unlock(&(manager->mutex));

  printf("\n ****signal to threads***.\n");
  pthread_mutex_lock(&(manager->mutex));
  {
    while (manager->number_of_threads != 0)
    {
      pthread_cond_wait(&(manager->conditional_variable), &(manager->mutex));
      usleep(100);
    }
    pthread_cond_destroy(&(manager->conditional_variable));
  }

  usleep(1000);
  pthread_mutex_unlock(&(manager->mutex));

  pthread_mutex_destroy(&(manager->mutex));
}

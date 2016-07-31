#include "request_list.h"

void init_node(request_list_node *this,
               FILE *file,
               char *buffer,
               uint32_t data_size,
               uint8_t operation)
{
  this->file      = file;
  this->buffer    = buffer;
  this->data_size = data_size;\
  this->operation = operation;
}


request_list_node create_request(FILE *file,
                                 char *buffer,
                                 uint32_t data_size,
                                 uint8_t operation)
{
  request_list_node node;
  init_node(&node, file, buffer, data_size, operation);
  return node;
}


void destroy_node(request_list_node *this)
{
  this->file      = NULL;
  this->buffer[0] = '\0';
  this->operation = None;
}

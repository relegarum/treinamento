#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_utils.h"
const char const* IndexStr = "/index.html";

int8_t verify_file_path(char *path, char *resource, char *fullpath)
{
  int32_t resource_size = strlen(resource);
  if( strncmp(resource, "/", resource_size) == 0 ||
      strncmp(resource, ".", resource_size) == 0 )
  {
    strncpy(resource, IndexStr, strlen(IndexStr));
  }

  // build string
  resource_size = strlen(resource);
  const int32_t path_size      = strlen(path);
  const int32_t file_name_size = path_size + resource_size + 1;
  char real_path[PATH_MAX];
  memset(real_path, '\0', PATH_MAX);
  snprintf(fullpath, file_name_size, "%s%s", path, resource);
  if  (realpath(fullpath, real_path) != NULL )
  {
    if (strncmp(path, real_path, path_size) != 0)
    {
      printf("Directory not found\n");
      goto clear_full_path;
    }
  }
  else
  {
    printf("Directory not found\n");
    goto clear_full_path;
  }

  memset(fullpath, '\0', PATH_MAX);
  strncpy(fullpath, real_path, strlen(real_path) + 1);
  return 0;

clear_full_path:
  memset(fullpath, '\0', PATH_MAX);
  return 1;
}


int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime)
{
  char *cmd_mask = "file -i %s";
  int32_t total_size = strlen(cmd_mask) + full_path_size;
  char *cmd = malloc(sizeof(char)*(total_size));
  snprintf(cmd, total_size, cmd_mask, full_path);
  FILE* pipe = popen(cmd, "r");
  if (pipe == NULL)
  {
    return -1;
  }

  fscanf(pipe, "%*s %[^\n]s\n", mime);
  pclose(pipe);
  return 0;
}


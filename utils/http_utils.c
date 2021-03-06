/* \file http_utils.c
 *
 * \brief Contem a implementacao de utilitarios relativos ao protocolo HTTP 1.0
 *
 * "$Id: $"
*/
#include "http_utils.h"
#include "file_utils.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#define MAX_REQUEST_SIZE      8000
#define MAX_ERROR_STR_SIZE    30
#define MAX_REQUEST_MASK_SIZE 23 /* GET %s HTTP/1.0\r\n\r\n */
#define PROTOCOL_SIZE         9

const char *HtmlBadRequestFileName   = "BadRequest.html";
const char *HtmlNotFoundFileName     = "NotFound.html";
const char *HtmlInternalErrorName    = "InternalErrorName.html";
const char *HtmlUnauthorizedFileName = "Unauthorized.html";
const char *HtmlWrongVersionFileName = "WrongVersion.html";
const char *HtmlNotImplementedFileName       = "NotImplemented.html";
const char *HtmlForbiddenFileName            = "Forbidden.html";
const char *HTTP10Str                = "HTTP/1.0";
const char *HTTP11Str                = "HTTP/1.1";

FILE *bad_request_file     = NULL;
FILE *not_found_file       = NULL;
FILE *internal_error_file  = NULL;
FILE *unauthorized_file    = NULL;
FILE *wrong_version_file   = NULL;
FILE *not_implemented_file = NULL;
FILE *forbidden_file       = NULL;

uint32_t   g_id = 0;

#define HTML_HEADER(number, string) "HTTP/1.0 "#number" "#string"\r\n";
#define HTML_ERROR(number, string) "<HTML><TITLE>"#number" "#string"</TITLE>\
                                    <BODY><H2>"#number" "#string"</H2></BODY>\
                                    </HTML>"

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

uint32_t get_response_size(char *first_chunk)
{
  char *slice = strstr(first_chunk, "Content-Length:");
  uint32_t size = 0;
  if (slice != NULL)
  {
    sscanf(slice, "Content-Length: %d\r\n", &size);
  }

  return size;
}

uint32_t handle_response_status(char *http_response)
{
  char error_string[MAX_ERROR_STR_SIZE];
  int32_t status = 0;

  if (sscanf(http_response,"HTTP/%*s %3d %29s\r\n", &status, error_string) != 2)
  {
    printf("Coudn't parse HTTP Status\n");
    return -1;
  }

  switch (status)
  {
  case Ok:
      {
        return 0;
      }
      break;
  case Not_Found:
      {
        printf("Resource not found in server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case MovedPermanently:
      {
        printf("Resource was permanently move from the server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case MovedTemporarily:
      {
        printf("Resource was temporarily move from the server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case ServerError:
      {
        printf("Server error. Status: %s\n", error_string);
        return -1;
      }
      break;
  default:
      {
        printf("Unknown error status. Status: %s\n", error_string);
        return -1;
      }
      break;
  }
}

/**/
int32_t get_header(int socket_descriptor, char *resource_required, int32_t *header_length, int32_t *content_length)
{
  uint32_t resource_required_length = strlen(resource_required);
  uint32_t request_msg_size         = MAX_REQUEST_MASK_SIZE +
                                      ((resource_required_length != 0) ? resource_required_length :
                                                                         strlen(IndexStr));

  char *request_msg = malloc(sizeof(char)*request_msg_size);

  snprintf(request_msg, request_msg_size, RequestMsgMask, (resource_required_length != 0) ? resource_required : IndexStr);
  int32_t request_len = strlen(request_msg);
  int32_t bytes_sent = send(socket_descriptor, request_msg, request_len, 0);

  free(request_msg);
  if (bytes_sent != request_len)
  {
    printf("Coudn't send entire request\n");
    return 1;
  }

  char header_buffer[300];
  int bytes_received = recv(socket_descriptor, header_buffer, sizeof(header_buffer), 0);
  if (bytes_received <= 0)
  {
    printf("Coudn't receive response\n");
    return 1;
  }
  header_buffer[bytes_received] = '\0';

  *header_length  = bytes_received;
  *content_length = get_response_size(header_buffer);

  printf( "Header Received:\n %s \n", header_buffer );

  return 0;
}

void get_resource(char *uri, char *hostname, char *resource)
{
  const char *protocol_suffix_end = "://";
  char *pointer = strstr(uri, protocol_suffix_end);
  if (pointer != NULL)
  {
    pointer += strlen( protocol_suffix_end );
    sscanf(pointer, "%[^/]%s", hostname, resource);
  }
  else
  {
    sscanf(uri, "%[^/]%s", hostname, resource);
  }
}

int32_t download_file(int socket_descriptor,
                      char *hostname,
                      char *resource_required,
                      int32_t transmission_rate,
                      FILE *output_file)
{
  uint32_t resource_required_length = strlen(resource_required);
  uint32_t hostname_length          = strlen(hostname);

  const char *request_mask = "GET %s HTTP/1.0\r\n"
                             "Host: %s\r\n"
                             "User-Agent: AkerClient\r\n"
                             "\r\n";
  uint32_t request_mask_length = strlen(request_mask);
  uint32_t request_total_size = request_mask_length +
                               ((resource_required_length != 0) ? resource_required_length : strlen(IndexStr))+
                               hostname_length +
                               1;

  char *request_msg = malloc(sizeof(char)*(request_total_size));
  snprintf(request_msg,
           request_total_size,
           request_mask,
           (resource_required_length != 0) ? resource_required : IndexStr,
           hostname );

  printf( "%s\n", request_msg );

  int32_t request_len = strlen(request_msg);
  int32_t total_bytes_sent = 0;
  do
  {
    int32_t bytes_sent = 0;
    int32_t attempt_size = request_len - total_bytes_sent;
    bytes_sent = send(socket_descriptor, &request_msg[total_bytes_sent], attempt_size, 0);
    if (bytes_sent == -1)
    {
        perror(__FUNCTION__);
        return -1;
    }
    total_bytes_sent += bytes_sent;
  }
  while (total_bytes_sent != request_len);
  free(request_msg);

  int32_t header_length  = 0;
  int32_t content_length = 0;
  if (handle_header( socket_descriptor, &header_length, &content_length ) == -1)
  {
    return -1;
  }

  char *header = malloc(sizeof(char)*header_length);
  char *carriage = header;
  int32_t bytes_received = recv(socket_descriptor, carriage, header_length, 0);
  if (bytes_received < header_length)
  {
    carriage = &header[ bytes_received ];
    bytes_received = recv( socket_descriptor, carriage, ( header_length - bytes_received), 0);
  }
  free( header );

  bytes_received = 0;
  char *chunk = malloc(sizeof(char)*transmission_rate);
  while ((bytes_received = recv(socket_descriptor, chunk, transmission_rate, 0)) != 0)
  {
    int write_bytes = transmission_rate;
    if (bytes_received < transmission_rate)
    {
      chunk[ bytes_received ] = '\0';
      write_bytes = bytes_received;
    }

    fwrite(chunk, sizeof(char), write_bytes, output_file);
  }

  free(chunk);

  return 0;
}

int32_t extract_content(char *http_response, char *content, int32_t content_length)
{
  char *pointer    = strstr( http_response, EndOfHeader);
  char *contentPtr = pointer + END_OF_HEADER_SIZE; /*strlen( \r\n\r\n )*/
  strncpy( content, contentPtr, content_length );

  return 0;
}

int32_t handle_header(int socket_descriptor, int32_t *header_length, int32_t *content_size )
{
  int32_t bytes_received       = 0;
  const int32_t header_slice_size = 2048;
  char header_slice[header_slice_size];
  bytes_received = recv(socket_descriptor, header_slice, header_slice_size, MSG_PEEK );
  if (bytes_received == 0)
  {
    printf( "Nothing was received as a Header!" );
    return -1;
  }

  if (handle_response_status( header_slice ) != 0)
  {
    return -1;
  }

  char *pointer    = strstr( header_slice, EndOfHeader);
  char *contentPtr = pointer + END_OF_HEADER_SIZE; /*strlen( \r\n\r\n )*/
  *header_length   = (contentPtr - header_slice);
  *content_size    = get_response_size( header_slice );

  return 0;
}

int verify_connection(ConnectionManager *manager,
                      int32_t listening_socket,
                      fd_set *read_fds,
                      fd_set *master,
                      int *greatest_fds)
{
  struct sockaddr_storage client_address;

  if (FD_ISSET(listening_socket, read_fds))
  {
    socklen_t addrlen = sizeof(client_address);
    int32_t new_socket_description = accept(listening_socket,
                                    (struct sockaddr *)&client_address,
                                    &addrlen);
    if (new_socket_description == -1)
    {
      perror("Accept");
      return -1;
    }
    else
    {
      Connection* item = create_connection_item(new_socket_description, ++g_id);
      add_connection_in_list(manager, item);

      handle_new_socket( new_socket_description, master, greatest_fds);
      if (set_socket_as_nonblocking(new_socket_description) != 0)
      {
        perror("set as nonblock");
        return -1;
      }

      //char remote_ip[INET6_ADDRSTRLEN];
      /*inet_ntop(client_address.ss_family,
                get_in_addr((struct sockaddr *)&client_address),
                remote_ip,
                sizeof(remote_ip));*/

      //printf("Connection from %s -> socket_num = %d\n", remote_ip, new_socket_description);
    }
  }
  return 0;
}

int8_t create_file(FILE **file,
                   char *html,
                   char *base_path,
                   const int32_t path_length,
                   const char *file_name)
{
  char full_path[PATH_MAX];
  snprintf(full_path, path_length + strlen(html) + 2, "%s/%s", base_path, file_name);
  *file = fopen(full_path, "w+b");
  if (*file != NULL)
  {
    fwrite(html, sizeof(char), strlen(html), *file);
    fflush(*file);
  }
  else
  {
    perror(__FUNCTION__);
    printf("filename %s\n", file_name);
    return -1;
  }

  return 0;
}

void free_file(FILE **file)
{
  if (*file != NULL)
  {
    fclose(*file);
    *file = NULL;
  }
}

void create_default_response_files(char *path)
{
  printf("path %s", path);
  int32_t path_length = strlen(path);
  create_file(&bad_request_file,     HTML_ERROR(400, Bad Request),                path, path_length, HtmlBadRequestFileName);
  create_file(&internal_error_file,  HTML_ERROR(500, Internal Server Error),      path, path_length, HtmlInternalErrorName);
  create_file(&not_found_file,       HTML_ERROR(404, Not Found),                  path, path_length, HtmlNotFoundFileName);
  create_file(&unauthorized_file,    HTML_ERROR(401, Unauthorized),               path, path_length, HtmlUnauthorizedFileName);
  create_file(&wrong_version_file,   HTML_ERROR(505, HTTP Version Not Supported), path, path_length, HtmlWrongVersionFileName);
  create_file(&not_implemented_file, HTML_ERROR(501, HTTP Not Implemented),       path, path_length, HtmlNotImplementedFileName);
  create_file(&forbidden_file,       HTML_ERROR(403, HTTP Forbidden),             path, path_length, HtmlForbiddenFileName);
}

void clean_default_files()
{
  free_file(&bad_request_file);
  free_file(&not_found_file);
  free_file(&internal_error_file);
  free_file(&unauthorized_file);
  free_file(&wrong_version_file);
  free_file(&not_implemented_file);
  free_file(&forbidden_file);
}


int setup_listening_connection(char* port, int32_t* listening_socket)
{
  int success = 0;
  struct addrinfo *servinfo = NULL;
  const int32_t    true_value      = 1;
  struct addrinfo  hints;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((success = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
  {
    printf("Error in getaddrinfo: %s\n", gai_strerror(success));
    success = -1;
    goto exit_setup_listening;
  }

  struct addrinfo *serverinfo_ptr    = NULL;
  // Get valid socket to listen
  for (serverinfo_ptr = servinfo;
       serverinfo_ptr != NULL;
       serverinfo_ptr = serverinfo_ptr->ai_next)
  {
    if ((*listening_socket = socket(serverinfo_ptr->ai_family,
                                   serverinfo_ptr->ai_socktype,
                                   serverinfo_ptr->ai_protocol)) == -1)
    {
      perror("Server socket\n");
      continue;
    }

    if ((success = setsockopt(*listening_socket,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              &true_value,
                              sizeof(true_value))) == -1)
    {
      perror("setsockopt");
      success = -1;
      goto exit_setup_listening;
    }

    if (bind(*listening_socket,
             serverinfo_ptr->ai_addr,
             serverinfo_ptr->ai_addrlen) == -1)
    {
      close(*listening_socket);
      perror("server bind");
      continue;
    }

    break;
  }

  if (serverinfo_ptr == NULL)
  {
    printf("Failed to bind\n");
    success = -1;
    goto exit_setup_listening;
  }

exit_setup_listening:

  if (servinfo != NULL )
  {
    freeaddrinfo(servinfo);

    servinfo       = NULL;
    serverinfo_ptr = NULL;
  }

  return 0;
}

int32_t verify_protocol(char *protocol)
{
  if ((strncmp(protocol, HTTP10Str, PROTOCOL_SIZE) != 0) &&
      (strncmp(protocol, HTTP11Str, PROTOCOL_SIZE) != 0) )
  {
    return -1;
  }

  return 0;
}

int32_t set_socket_as_nonblocking(int socket_descriptor)
{
  if (fcntl(socket_descriptor, F_SETFL, fcntl(socket_descriptor, F_GETFL) | O_NONBLOCK) < 0)
  {
    perror("fcntl");
    return -1;
  }

  return 0;
}

int32_t set_socket_timeout(int socked_descriptor, struct timeval *timeout)
{
  if (setsockopt(socked_descriptor, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) < 0)
  {
    perror("Error");
    return -1;
  }

  return 0;
}

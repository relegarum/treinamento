#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void get_resource( char *uri, char *hostname, char *resource )
{
  sscanf( uri, "%[^/]%s", hostname, resource );
}

int handle_arguments( int argc,
                      char **argv,
                      struct addrinfo **server_info,
                      char *resource_required,
                      FILE **output )
{
  const int32_t number_of_elements = 3;
  if ( argc < number_of_elements )
  {
    printf( "wrong number of arguments" );
    exit( 1 );
  }

  struct addrinfo hints, *res;
  char ipstr[ INET6_ADDRSTRLEN ];

  memset( &hints, 0, sizeof( hints ) );
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  char hostname[ 50 ];
  get_resource( argv[ 1 ], hostname, resource_required );

  int32_t status = getaddrinfo( hostname, "80", &hints, &res );
  if ( status != 0 )
  {
    printf( "getaddrinfo: %s\n", gai_strerror( status ) );
    exit( 1 );
  }

  printf( "IP adresses for %s :", hostname );

  *server_info = res;
  struct addrinfo *p;
  for ( p = res; p != NULL; p = p->ai_next )
  {
    void *addr;
    char ipver[ 50 ];

    if ( p->ai_family == AF_INET )
    {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &( ipv4->sin_addr );
      strncpy( ipver, "IPv4", 5 );
    }
    else
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &( ipv6->sin6_addr );
      strncpy( ipver, "IPv6", 5 );
    }

    inet_ntop( p->ai_family, addr, ipstr, sizeof( ipstr ) );
    printf( " %s: %s\n", ipver, ipstr );
  }


  if ( ( argc == 4 ) &&
       ( strncmp( argv[ 3 ], "over", 4 ) == 0 ) )
  {
    *output = fopen( argv[ 1 ], "w" );
  }
  else
  {
    struct stat buffer;
    int result = stat( argv[ 1 ], &buffer );
    if( result != 0 )
    {
      printf( "File doesn't exist\n" );
      exit( 1 );
    }
    else
    {
      *output = fopen( argv[ 1 ], "w" );
    }
  }

  if ( *output == NULL )
  {
    printf( "Coudn't open file: %s", argv[ 1 ] );
    exit( 1 );
  }

  return 0;
}

int main( int argc, char **argv )
{
  struct addrinfo *server_info = NULL;
  FILE            *output_file  = NULL;
  char		         resource_required[ 50 ];

  if ( handle_arguments( argc, argv, &server_info, resource_required, &output_file ) != 0 )
  {
    printf( "Couldn't handle arguments\n" );
    return 1;
  }

  int socket_descriptor = socket( server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol );
  if ( socket_descriptor == -1 )
  {
    perror( "socket" );
    exit( 1 );
  }

  int status = connect( socket_descriptor, server_info->ai_addr, server_info->ai_addrlen );
  if ( status == -1 )
  {
    perror( "connect" );
    exit( 1 );
  }

  printf( "Connected!\n" );

  char request_msg[ 80 ];
  sprintf( request_msg, "GET %s HTTP/1.0\r\n\r\n", resource_required );
  int32_t request_len = strlen( request_msg );
  int32_t bytes_sent = send( socket_descriptor, request_msg, request_len, 0 );
  if ( bytes_sent != request_len )
  {
    printf( "Coudn't send entire request\n" );
    return 1;
  }

  char buffer[ 5000 ];
  int bytes_received = recv( socket_descriptor, buffer, sizeof( buffer ), 0 );
  if( bytes_received <= 0 )
  {
    printf( "Coudn't receive response\n" );
    return 1;
  }
  buffer[ bytes_received ] = '\0';

  printf( "Received:\n %s \n", buffer );

  freeaddrinfo( server_info );
  fclose( output_file );
  return 0;
}


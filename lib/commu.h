#include "util.h"
//
/*---------------------------------------------------------------------
   read a block data length is 'len'.
   return < 0, if error occured, and
          = 0, eof, and
          = 1, write ok.
          = 2, timeout.
---------------------------------------------------------------------*/
int read_blk( int fd, byte* ptr, int len, int timeout );
int read_str( int fd, char* ptr, int len, int timeout );

/*---------------------------------------------------------------------
   write a block data length is 'len'.
   return <  0, if error occured, and
          == 0, timeout
          >  0, write size (== len).
---------------------------------------------------------------------*/
int write_blk( int fd, byte *data, int len, int sec );

/*---------------------------------------------------------------------
   write a message length is 'len', and get an ack from remote.
   return <= 0, if error occured, and
          == 1, write ok.
          == 2, timeout.
          == 3, bad data.
---------------------------------------------------------------------*/
int write_msg( int fd, byte* data, int len, int timeout );

/*---------------------------------------------------------------------
   read a message length is '*len', and write an ack to remote.
   return <= 0, if error occured or eof, and
          == 1, write ok.
          == 2, timeout.
          == 3, bad data.
---------------------------------------------------------------------*/
int read_msg( int fd, byte** data, int* len, int timeout );

int socket_check(int fd, int sec, int usec);

/*---------------------------------------------------------------------
   server prepare for 'port' and return socket file desc.
   return <  0, if error occured, and
          >= 0, server socket fd.
---------------------------------------------------------------------*/
int server_prepare(int port);
int server_prepare_unix(char *path);
int server_listen(int sockfd, int backlog);

/*---------------------------------------------------------------------
   server wait for 'tv_sec' seconds by 'sockfd' for client program.
   return = 0, if timeout, and
          < 0, if error occured, and
          > 0, new socket fd.
---------------------------------------------------------------------*/
int server_wait( int sockfd, int tv_sec );
int client_open( char* host, int port ) ;
int client_open_unix( char* path ) ;

extern int fd_is_writable(int) ;
extern int fd_is_readable(int) ;

typedef
struct bio4_rec
{
  int	fd ;
  int	num ;
  char	buffer[4] ;
}
bio4T ;

void bio4_write_sync(bio4T *bio) ;
void bio4_write(bio4T *bio, char *data) ;

char * get_remote_host(int fd, char *remote_host);

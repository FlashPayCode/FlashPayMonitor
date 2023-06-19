/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
*/
//testgit20210106

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN_NT
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#define FD_SETSIZE      1024
#include <windows.h>
#include <winsock.h>
#include <io.h>
#endif

/*
#include "Maverick.h"
*/

#ifdef WIN32
#define bzero(a,b)      memset((a),0,(b))
#endif

#define	ACK	'\0'
#define	BAD	'\1'

typedef          short int2  ;

/*
#ifndef ulong
typedef unsigned long  ulong  ;
#endif
*/

#include "util.h"

int  fd_is_writable(int fd)
{
  fd_set wfds;
  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = 1;
  FD_ZERO(&wfds);
  FD_SET(fd,&wfds);
  if ( select(FD_SETSIZE,NULL,&wfds,NULL,&tv) == 1 ) return 1 ;
  return 0 ;
}
                
int  fd_is_readable(int fd)
{
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = 1;
  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);
  if ( select(FD_SETSIZE,&rfds,NULL,NULL,&tv) == 1 ) return 1 ;
  return 0 ;
}

/*---------------------------------------------------------------------
   read a block data length is 'len'.
   return < 0, if error occured, and
          = 0, eof, and
          = 1, write ok.
          = 2, timeout.
---------------------------------------------------------------------*/
int read_blk( int fd, byte* ptr, int len, int sec )
{
  int	res, rlen, pos = 0 ;
  fd_set rfds;
  struct timeval tv;
  while ( sec > 0 )
  {
    tv.tv_sec  = 1;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    res = select(fd+1,&rfds,NULL,NULL,&tv) ;
    if (res <  0) return -1;
    if (res == 1)
    {
      rlen = recv( fd, ptr+pos, len-pos, 0 ) ;
      if ( rlen <= 0 )
      {
        if ( errno != EAGAIN ) return -1 ;
        usleep(1);
      }
      else pos += rlen;
      if (pos == len) return len;
    }
    sec--;
  }
  return 0;
}

int read_str( int fd, char* ptr, int len, int /*timeout*/ )
{
  int    res = 0, first = 1, pos = 0 ;
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec  = 1;
  tv.tv_usec = 0;
  while ( len > 0 )
  {
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    res = select(FD_SETSIZE,&rfds,NULL,NULL,&tv) ;
    if ( res == 1 )
    {
#ifdef WIN32
      res = recv( fd, ptr+pos, 1, 0 ) ;
#else
      res = read( fd, ptr+pos, 1 ) ;
#endif
      if ( res <= 0 )
      {
        return 0 ;
      }
      if ( first )
      {
        first = 0 ;
        if ( *(ptr+pos) != 'm' ) return 0 ;
      }
      if ( *(ptr+pos) == 0x0a ) break ;
      pos++; len--;
    }
    else
    {
      return 0 ;
    }
    tv.tv_sec  = 0;
    tv.tv_usec = 500000;
  }
  *(ptr+pos) = '\0' ;
  return 1 ;
}
                                                                                
/*---------------------------------------------------------------------
   write a block data length is 'len'.
   return <= 0, if error occured, and
          == 1, write ok.
          == 2, timeout.
---------------------------------------------------------------------*/

int write_blk(int fd, byte *data, int len, int sec )
{
  int	 res, rlen, pos = 0 ;
  fd_set wfds;
  struct timeval tv;
  while (sec > 0)
  {
    tv.tv_sec  = 1;
    tv.tv_usec = 0;
    FD_ZERO(&wfds);
    FD_SET(fd,&wfds);
    res = select(fd+1, NULL, &wfds, NULL, &tv) ;
    if (res <  0) return -1;
    if (res == 1)
    {
      rlen = send(fd, data + pos, len - pos, 0 ) ;
      if ( rlen <= 0 )
      {
        if ( errno != EAGAIN ) return -1 ;
        usleep(1);
      }
      else pos += rlen;
      if (pos == len) return len;
    }
    sec--;
  }
  return 0;
}

int socket_check(int fd, int sec, int usec )
{
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);
  return select(fd+1,&rfds,NULL,NULL,&tv) ;
}
                  
/*---------------------------------------------------------------------
   server prepare for 'port' and return socket file desc.
   return <  0, if error occured, and
          >= 0, server socket fd.
---------------------------------------------------------------------*/
int server_prepare(int port)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( sockfd >= 0)
  {
    int on = 1 ;
    struct sockaddr_in sin ;
    bzero(&sin, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY) ;
    sin.sin_port        = htons(port);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) ;
    if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
      return -1 ;
    /* Marked By GEORGE, 2015/06/15
    listen(sockfd, 64);
    */
    return sockfd ;
  }
  else printf( "sockfd < 0\n" ) ;
  return -1 ;
}

int server_listen(int sockfd, int backlog)
{
  return listen(sockfd, backlog);
}

int server_prepare_unix(char *path)
{
  int sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if ( sockfd >= 0)
  {
    int on = 1 ;
    struct sockaddr_un address ;
    size_t addrLength ;
    
    unlink(path) ;
    
    address.sun_family      = AF_UNIX;
    strcpy(address.sun_path,path) ;

    addrLength = sizeof(address.sun_family) + strlen(address.sun_path) ;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) ;

    if (bind(sockfd, (struct sockaddr *)&address,addrLength) != 0)
      return -1 ;
      
    listen(sockfd, 64);
    return sockfd ;
  }
  else printf( "sockfd < 0\n" ) ;
  return -1 ;
}

/*---------------------------------------------------------------------
   server wait for 'tv_sec' seconds by 'sockfd' for client program.
   return = 0, if timeout, and
          < 0, if error occured, and
          > 0, new socket fd.
---------------------------------------------------------------------*/
int server_wait( int sockfd, int tv_sec )
{
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec  = tv_sec;
  tv.tv_usec = 0;
  FD_ZERO(&rfds);
  FD_SET(sockfd, &rfds);
  
  if ( select(FD_SETSIZE, &rfds, NULL, NULL, &tv) == 1 )
  {
    struct sockaddr_in sin ;
    int s, len = sizeof(sin);

    s = accept(sockfd, (struct sockaddr *)&sin, (socklen_t*)&len);
    return s ;
  }
  return 0 ;
}

/*
printf( "mbrcli: connect() fail, code = " ) ;
    if ( errno == EADDRNOTAVAIL ) printf( "EADDRNOTAVAIL\n" ) ;
    if ( errno == EAFNOSUPPORT ) printf( "EAFNOSUPPORT\n" ) ;
    if ( errno == EALREADY ) printf( "EALREADY\n" ) ;
    if ( errno == EBADF ) printf( "EBADF\n" ) ;
    if ( errno == ECONNREFUSED ) printf( "ECONNREFUSED\n" ) ;
    if ( errno == EINPROGRESS ) printf( "EINPROGRESS\n" ) ;
    if ( errno == EINTR ) printf( "EINTR\n" ) ;
    if ( errno == EISCONN ) printf( "EISCONN\n" ) ;
    if ( errno == ENETUNREACH ) printf( "ENETUNREACH\n" ) ;
    if ( errno == ENOTSOCK ) printf( "ENOTSOCK\n" ) ;
    if ( errno == EPROTOTYPE ) printf( "EPROTOTYPE\n" ) ;
    if ( errno == ENOTDIR ) printf( "ENOTDIR\n" ) ;
    if ( errno == ENAMETOOLONG ) printf( "ENAMETOOLONG\n" ) ;
    if ( errno == ETIMEDOUT ) printf( "ETIMEDOUT\n" ) ;
    if ( errno == EACCES ) printf( "EACCES\n" ) ;
    if ( errno == EIO ) printf( "EIO\n" ) ;
    if ( errno == EADDRINUSE ) printf( "EADDRINUSE\n" ) ;
    if ( errno == ECONNRESET ) printf( "ECONNRESET\n" ) ;
    if ( errno == EHOSTUNREACH ) printf( "EHOSTUNREACH\n" ) ;
    if ( errno == EINVAL ) printf( "EINVAL\n" ) ;
    if ( errno == ENAMETOOLONG ) printf( "ENAMETOOLONG\n" ) ;
    if ( errno == ENETDOWN ) printf( "ENETDOWN\n" ) ;
    if ( errno == ENOBUFS ) printf( "ENOBUFS\n" ) ;
    if ( errno == ENOSR ) printf( "ENOSR\n" ) ;
    if ( errno == EOPNOTSUPP ) printf( "EOPNOTSUPP\n" ) ;
*/

int client_open( char* host, int port )
{
  int  finet = -1;
  struct sockaddr_in sout ;
  struct hostent *h ;
  h = gethostbyname( host ) ;
  if ( h == NULL ) return -1 ;
  bzero(&sout, sizeof(sout));
  sout.sin_family      = AF_INET;
  sout.sin_addr.s_addr = *(unsigned long int*)(h->h_addr) ;
  sout.sin_port        = htons(port);
  if ( ( finet = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) return -2 ;
  if (connect(finet, (struct sockaddr *)&sout, sizeof(sout)) < 0)
  {
    close (finet) ;
    return -3 ;
  }
  /*
  strcpy(last_host, host) ;
  last_port = port ;
  */
  return finet ;
}

int client_open_unix( char* path )
{
  int    sock ;
  struct sockaddr_un address ;
  size_t addrLength ;

  if ( ( sock = socket(PF_UNIX, SOCK_STREAM, 0) ) < 0 ) return -2 ;
  
  address.sun_family      = AF_UNIX;
  strcpy(address.sun_path,path) ;
  
  addrLength = sizeof(address.sun_family) + strlen(address.sun_path) ;

  if (connect(sock, (struct sockaddr *)&address, addrLength) < 0)
  {
    close (sock) ;
    return -3 ;
  }
  return sock ;
}

/* Buffered IO of 4 bytes.
------------------------------------------------------------------*/
#include "commu.h"

static void bio4_write_sync1(bio4T *bio)
{
  if ( bio->num )
  {
    int index = 4 - bio->num ;
printf( "bio4_write_sync(), index = %d\n", index) ;
fflush(stdout) ;
    if ( fd_is_writable(bio->fd) )
    {
      write(bio->fd,&(bio->buffer[index]),1) ;
      bio->num -- ;
    }
  }
}

void bio4_write_sync(bio4T *bio)
{
  if ( bio->num == 0 ) return ;
  bio4_write_sync1(bio) ;
  bio4_write_sync1(bio) ;
  bio4_write_sync1(bio) ;
  bio4_write_sync1(bio) ;
}

void bio4_write(bio4T *bio, char *data)
{
  if ( bio->num )
  {
    bio4_write_sync(bio) ;
    if ( bio->num ) return ;
  }

printf( "bio4_write()\n") ;
fflush(stdout) ;

  memcpy(bio->buffer,data,4) ;
  bio->num = 4 ;
  bio4_write_sync(bio) ;
}

char * get_remote_host(int fd, char *remote_host)
{
  struct sockaddr peeraddr ;
  int    peersize = sizeof(peeraddr) ;
  int    ret ;

  ret = getpeername(fd,(struct sockaddr*)&peeraddr,(socklen_t*)&peersize);
  remote_host[0] = '\0';
  if ( ret == 0 )
  {
    sprintf(remote_host,"%d.%d.%d.%d",
	(unsigned char)peeraddr.sa_data[2],
	(unsigned char)peeraddr.sa_data[3],
	(unsigned char)peeraddr.sa_data[4],
	(unsigned char)peeraddr.sa_data[5] ) ;
  }
  return remote_host ;
}



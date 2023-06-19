#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "util.h"

#define SA      struct sockaddr
#define MAXLINE 4096
#define LISTENQ	1024

extern int h_errno;
static	int 	http_port = 80;
static	char	szInputBuf[32767];
static	char	sendline[MAXLINE + 1];
static	char	recvline[MAXLINE + 1];

char *process_http(int sockfd, char *host, char *page, char *poststr)
{
  ssize_t n, recv_count = 0;
  sprintf(sendline, "POST %s HTTP/1.0\r\n"
                    "Host: %s\r\n"
                    "Content-type: application/json\r\n"
                    "Content-length: %d\r\n\r\n"
                    "%s", page, host, (int)strlen(poststr), poststr);
  write(sockfd, sendline, strlen(sendline));

  char *szPtr = szInputBuf;
  while ((n = read(sockfd, recvline, MAXLINE)) > 0)
  {
    memcpy(szPtr,recvline,n);
    szPtr += n;
    recv_count += n;
recvline[n] = '\0';
printf("%s", recvline);
fflush(stdout);
  }
  szPtr[0] = '\0';

  szPtr = strstr(szInputBuf,"\r\n\r\n");
  if (szPtr)
  {
    szPtr += 4;  
printf("%s\n", szPtr);
fflush(stdout);
  }
  return szPtr; //recv_count;
}

void Html_Port(int iPort)
{
  http_port = iPort;
}

char *Html_Post(const char *url, char *poststr)
{
  char hname[256], page[256];
  int sockfd;
  struct sockaddr_in servaddr;
  char **pptr;
  
  ParseUrl(url, hname, page);
  
  char str[50];
  struct hostent *hptr;
  if ((hptr = gethostbyname(hname)) == NULL)
  {
    return NULL;
printf("ERROR: gethostbyname failed for host: %s: %s", hname, hstrerror(h_errno));
fflush(stdout);
  }
printf("hostname: %s\n", hptr->h_name);
fflush(stdout);
  if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL)
  {
printf("address: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
  }
  else
  {
printf("Error call inet_ntop \n");
fflush(stdout);
    return NULL;
  }
 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(http_port);
  inet_pton(AF_INET, str, &servaddr.sin_addr);

printf("connect...port = %d\n", http_port);
fflush(stdout);

  if (connect(sockfd, (SA *) & servaddr, sizeof(servaddr)) == 0)
  {

printf("connect ok\n");
fflush(stdout);

    char *result = process_http(sockfd, hname, page, poststr);
    close(sockfd);
    return result;
  }
printf("connect failed, errno = %d, %s\n", errno, strerror(errno));
fflush(stdout);

  close(sockfd);
  return NULL;
}

#ifndef RSOCKET_H
#define RSOCKET_H

#include <stdio.h>      // printf ....etc.
#include <string.h>     // string functions
#include <stdlib.h>     // exit(), free(), malloc ......etc.
#include <time.h>       // time(NULL)
#include <unistd.h>     // close
#include <sys/socket.h> // socket
#include <sys/select.h> // select 
#include <pthread.h>    // threadX
#include <signal.h>     // SIGKILL
#include <assert.h>
// To remove redundant headers


#define BUF_SIZE 100
#define MSG_SIZE 100
#define TABLE_SIZE 100
#define TIMEOUT 2
#define DROP_PROBALITY 0.3
#define SOCK_MRP 153

int dropMessage(float p);

void *threadX(void* param);

int r_socket(int domain, int type, int protocol);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int r_close(int fd);

#endif

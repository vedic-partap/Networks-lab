#ifndef RSOCKET_H
#define RSOCKET_H
/*
+++++++++++++ Vedic Partap +++++++++++++
+++++++++++++ 16CS10053 +++++++++++++
*/
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>  
#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <pthread.h> 
#include <signal.h>
#include <assert.h>
// To remove redundant headers

#define MSG_SIZE 100
#define TABLE_SIZE 100
#define TIMEOUT 2
#define DROP_PROBALITY 0.2
#define SOCK_MRP 153
#define BUFFER_SIZE 100

int dropMessage(float p);


int r_socket(int domain, int type, int protocol);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int r_close(int fd);

void *runnerX(void* param);
#endif

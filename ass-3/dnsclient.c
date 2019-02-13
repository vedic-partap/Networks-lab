/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 3

Client

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <netdb.h> 

#define PORT 8182
#define MAX_CHAR 100
#define MAX_LINE 1024
int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char hostname[MAX_CHAR], buffer[MAX_CHAR];
    int n;
    socklen_t len;
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        perror("[ERROR} Unable to create the socket\n");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    printf("Enter the hostname\n");
    scanf("%s",hostname);
    sendto(sockfd,(const char*)hostname,strlen(hostname)+1,0,
                    (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    len = sizeof(serv_addr);
    n = recvfrom(sockfd, (char *)buffer, MAX_LINE, 0,
                 (struct sockaddr *)&serv_addr, &len);
    buffer[n] = '\0';
    printf("IP for %s : %s\n",hostname,buffer);
    return 0;
}

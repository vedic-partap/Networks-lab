/*
+++++++++++++ Vedic Partap +++++++++++++
+++++++++++++ 16CS10053 +++++++++++++
*/
#include <stdio.h> 
#include <string.h>
#include <arpa/inet.h>
 
#include "rsocket.h"

#define MAXLINE 100 
#define ROLLNO 10053

struct sockaddr_in m2_addr; 
socklen_t len; 
char buffer[MAXLINE] ;

int main(int argc,  char **argv ) { 
  
    // Creating socket file descriptor 
    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(1); 
    } 
    scanf("%s",buffer);
    memset(&m2_addr, 0, sizeof(m2_addr)); 

    m2_addr.sin_family = AF_INET; 
    m2_addr.sin_port = htons(5000+2*ROLLNO); 
    m2_addr.sin_addr.s_addr = INADDR_ANY; 

    for(int i = 0;i<strlen(buffer);i++){  
        len = sizeof(m2_addr);
        r_sendto(sockfd, (const char *)(buffer+i), 1, 0,(const struct sockaddr *) &m2_addr, len);
    }
    r_close(sockfd); 
    return 0; 
} 


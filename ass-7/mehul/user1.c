#include <stdio.h> 
#include <string.h>
#include <arpa/inet.h>
 
#include "rsocket.h"

#define MAXLINE 100 
#define PORT 5000

struct sockaddr_in m2_addr; 
socklen_t len; 
char buffer[MAXLINE] ;

int main(int argc,  char **argv ) { 
  
    // Creating socket file descriptor 
    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    scanf("%s",buffer);
    memset(&m2_addr, 0, sizeof(m2_addr)); 

    m2_addr.sin_family = AF_INET; 
    m2_addr.sin_port = htons(PORT); 
    m2_addr.sin_addr.s_addr = INADDR_ANY; 

    // if (bind(sockfd, (const struct sockaddr *)&m2_addr, sizeof(m2_addr)) < 0 ) { 
    //     perror("bind failed"); 
    //     exit(EXIT_FAILURE); 
    // }
    
      
    for(int i = 0;i<strlen(buffer);i++){  
        len = sizeof(m2_addr);
        r_sendto(sockfd, (const char *)(buffer+i), 1, 0,(const struct sockaddr *) &m2_addr, len);
    }
    // while(1);
    r_close(sockfd); 
    return 0; 
} 


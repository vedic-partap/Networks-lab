#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MYPORT 3490
#define MAXBUFLEN 100
#define MAX_CHAR 100

int sock;
struct sockaddr_in server; 

void io_handler(int signal)
{
    int numbytes; 
    socklen_t addr_len = sizeof(server);
    char buf[MAXBUFLEN];

    if ((numbytes = recvfrom(sock, buf, MAXBUFLEN, 0,
                             (struct sockaddr *)&server, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("[%s]: %s \n", inet_ntoa(server.sin_addr), buf);
    return;
}

int main()
{
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("opening datagram socket");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(MYPORT);
    signal(SIGIO, io_handler);
    if (fcntl(sock, F_SETOWN,  getpid()) < 0)
    {
        perror("fcntl F_SETOWN");
        exit(1);
    }
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | FASYNC) < 0)
    {
        perror("fcntl F_SETFL, FASYNC");
        exit(1);
    }
    char message[MAX_CHAR];
    printf("Enter the message: ");
    scanf("%s",message);
    sendto(sock,message,strlen(message)+1,0,(struct sockaddr*)&server,sizeof(server));
    printf("Send\n");
    printf("Waiitng ...\n");
    pause();
    printf("Received\n");
    signal(SIGIO, SIG_DFL);
    return 0;
}
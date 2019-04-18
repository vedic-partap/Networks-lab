/* 
vedic partap

I would prefer non blocking because we have control over when we recive the data. While using Signal I/O we lose the control when will the IO will come.
Signal IO is difficult and messier to code. Also we have to take care of deadlock conditions.
The problem of recursive signal handler makes it a poorer choice. 


 */

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

int sock;
void io_handler(int signal)
{
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_in cli_addr; 
    socklen_t addr_len = sizeof(cli_addr);
    if ((numbytes = recvfrom(sock, buf, MAXBUFLEN, 0,
                             (struct sockaddr *)&cli_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("[%s]: %s \n", inet_ntoa(cli_addr.sin_addr), buf);
    printf("Sending ...\n");
    sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
    printf("Send\n");
    return;
}


int main()
{
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("opening datagram socket");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(MYPORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof server) < 0)
    {
        perror("binding datagram socket");
        exit(1);
    }

    signal(SIGIO, io_handler);
    if (fcntl(sock, F_SETOWN, getpid()) < 0)
    {
        perror("fcntl F_SETOWN");
        exit(1);
    }

    // third: allow receipt of asynchronous I/O signals
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | FASYNC) < 0)
    {
        perror("fcntl F_SETFL, FASYNC");
        exit(1);
    }

    printf("Waiting ...\n");
    pause();
    
    signal(SIGIO, SIG_DFL);
    return 0;
}

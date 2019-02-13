/*

Name : Vedic Partap
Roll No: 16CS10053
Assignment - 5

++++ Server Side ++++

*/
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CHAR 100
#define BUFFER_SIZE 20

// Function to send the int 
int sendInt(int num, int fd)
{
    int32_t conv = htonl(num);
    // printf("%d\n",conv);
    char *data = (char *)&conv;
    int left = sizeof(conv);
    int rc;
    do
    {
        rc = write(fd, data, left);
        if (rc < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // use select() or epoll() to wait for the socket to be writable again
            }
            else if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    return 0;
}

// Finding the size of the file in bytes
int fileSize(int fd)
{
    struct stat f_info;
    fstat(fd, &f_info);
    return (int)f_info.st_size;
}

// Read bytes in block of buffer_size and send
void sendFile(int fd, int sock_fd, int buffer_size)
{
    int byte_read = 0;
    char read_buffer[buffer_size];
    do
    {
        byte_read = read(fd, read_buffer, buffer_size - 1);
        read_buffer[byte_read] = '\0';
        send(sock_fd, read_buffer, strlen(read_buffer), 0);
    } while (byte_read == buffer_size - 1);
    return;
}

int main()
{
    int sock_fd,newsock_fd,fd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    if((sock_fd = socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sock_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to bind the socket\n");
        close(sock_fd);
        exit(1);
    }
    listen(sock_fd,5);
    while(1)
    {
        printf("\n>>> Ready to connect\n");
        cli_len = sizeof(cli_addr);
        newsock_fd = accept(sock_fd,(struct sockaddr*)&cli_addr,&cli_len);
        if (newsock_fd < 0)
        {
            perror("[ERROR] Unable to connect\n");
            continue;
        }
        else
        {
            printf("> Connected\n");
            char filename[MAX_CHAR];
            recv(newsock_fd, filename, MAX_CHAR, 0);
            printf("> File: %s\n", filename);
            fd = open(filename, O_RDONLY);
            if (fd < 0)
            {
                perror("[ERROR] File can't be found\n");
                send(newsock_fd, "E", 1, 0);
                close(newsock_fd);
                continue;
            }
            else
            {
                send(newsock_fd, "L", 1, 0);
                int filesize = fileSize(fd);
                sendInt(filesize, newsock_fd);
                sendFile(fd, newsock_fd, BUFFER_SIZE);
                printf("> Sent\n");
                close(newsock_fd);
                close(fd);
            }
            
        }        
    }
    return 0;
}
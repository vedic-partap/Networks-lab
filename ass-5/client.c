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

// Function to get int in network format
int get_int(int *num, int fd)
{
    int32_t ret;
    char *data = (char *)&ret;
    int left = sizeof(ret);
    int rc;
    do
    {
        rc = read(fd, data, left);
        if (rc <= 0)
        { /* instead of ret */
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // use select() or epoll() to wait for the socket to be readable again
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
    *num = ntohl(ret);
    return 0;
}

// Receive the file 
int getFile(int sock_fd, char * filename, int file_size,int buffer_size, int *X, int* Y)
{
    int cnt = file_size/buffer_size, last_size = file_size - buffer_size*cnt,total_bytes=0;
    int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644), i=0;
    char buffer[buffer_size+1];
    for(i=0;i<cnt;i++)
    {
        *Y = recv(sock_fd,buffer,buffer_size,MSG_WAITALL);
        total_bytes += *Y;
        buffer[buffer_size]='\0';
        write(fd,buffer,buffer_size);
        *X +=1;
    }
    if(last_size)
    {
        *Y = recv(sock_fd, buffer, last_size, MSG_WAITALL);
        total_bytes += *Y;
        buffer[last_size] = '\0';
        write(fd, buffer, last_size);
        *X+=1;
    }
    close(fd);
    return total_bytes;

}
int main()
{
    int sock_fd,status;
    struct sockaddr_in serv_addr;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM,0 )) < 0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if((status = connect(sock_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)))<0)
    {
        perror("[ERROR] Can't connect to the server\n");
        close(sock_fd);
        exit(1);
    }
    printf("Filename : ");
    char filename[MAX_CHAR];
    scanf("%s",filename);
    send(sock_fd,filename,strlen(filename)+1,0);
    char check[2];
    recv(sock_fd,check,1,0);
    check[1]='\0';
    if(!strcmp(check,"E"))
    {
        printf("File Can't be found\n");
        close(sock_fd);
        return 0;
    }
    else
    {
        int n;
        get_int(&n,sock_fd);
        printf("Size of file: %d\n",n);
        int total_blocks = 0,final_block_size = 0, total_bytes = 0;
        total_bytes =  getFile(sock_fd,"client.txt",n,BUFFER_SIZE,
                                &total_blocks,&final_block_size);
        printf("\n[SUCCESS] Received\n\n");
        printf("Buffer Size: %d\n",BUFFER_SIZE);
        printf("Total Blocks: %d\nFinal BLock Size: %d\nTotal Bytes Received: %d\n",
                total_blocks,final_block_size, total_bytes);
    }
    close(sock_fd);
    return 0;
}
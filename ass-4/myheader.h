#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <wait.h>

#define PORTX 50006
#define MAX_CHAR 100
#define MAX_TOKENS 10

/*
FUNCTION : intToByte
=====================
input : num (integer that has to be converted), char *a (to store the bytes)

+++++++++++++++++++++

It conver the inter to 4-byte charcter array. This function has not been used.
Just written to be used in case of sending int through chacacter array.

+++++++++++++++++++++

=====================

*/
void intToByte(int num, char* a)
{
    a[3] = (num >> 24) & 0xFF;
    a[2] = (num >> 16) & 0xFF;
    a[1] = (num >> 8) & 0xFF;
    a[0] = num & 0xFF;
}

/*
FUNCTION : ByesToInt
=====================
input : num (integer that has to be found), char *a (to store the bytes)

+++++++++++++++++++++

It convert the 4-byte charcter array to integer. This function has not been used.
Just written to be used in case of sending int through chacacter array.

+++++++++++++++++++++

=====================

*/
void bytesToInt(int *num, char a[4])
{
    *num = *(int *)a;
}

/*
FUNCTION : send_int
=====================
input : num (short integer that has to be send), fd socket ID

+++++++++++++++++++++

It connvet the short int to network level bits and then send it through TCP sockets.
This function is used in the main code to transfer the Erro Code and the length of the buffer.

+++++++++++++++++++++

Output: Error Code
=====================

*/
int send_int(int num, int fd)
{
    int16_t conv = htons(num);
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

/*
FUNCTION : get_int
=====================
input : fd socket ID, num (short integer that has to be send)

+++++++++++++++++++++

It receive the netwrok level bytes into the short int. 
This function is used in the main code to receive the error code and length of the buffer.

+++++++++++++++++++++

Output: Error Code
=====================

*/

int get_int(int fd, int *num)
{
    int16_t ret;
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
    *num = ntohs(ret);
    return 0;
}

/*
FUNCTION : __send_int
=====================
input : num (short integer that has to be send), fd socket ID

+++++++++++++++++++++

[NOT USED JUST FOR TESTING PURPOSE]
It connvet the short int to network level bits and then send it through TCP sockets.
This function is used in the main code to transfer the Erro Code and the length of the buffer.

+++++++++++++++++++++

Output: Error Code
=====================

*/
void __send_int(int x, int sock_fd)
{
    char buffer[MAX_CHAR], bytes[5];
    intToByte(x, bytes);
    strcpy(buffer, bytes);
    send(sock_fd, bytes, 5, 0);
    return;
}

/*
FUNCTION : __get_int
=====================
input : fd socket ID, num (short integer that has to be send)

+++++++++++++++++++++

[NOT USED JUST FOR TESTING PURPOSE]
It receive the network level bytes into the short int. 
This function is used in the main code to receive the error code and length of the buffer.

+++++++++++++++++++++

Output: Error Code
=====================

*/

void __get_int(int sock_fd, int *x)
{
    char buf[2], buffer[MAX_CHAR];
    int idx = 0;
    for (int i = 0; i < 5; i++)
    {
        recv(sock_fd, buf, 1, 0);
        buf[1] = '\0';
        buffer[idx++] = buf[0];
    }
    bytesToInt(x, buffer);
}

/*
FUNCTION : tokenise
=====================
input : command (string), tokens array of string

+++++++++++++++++++++

This will split the string using whitespace as delimter.

+++++++++++++++++++++

=====================

*/
void tokenise(char command[], char tokens[MAX_TOKENS][MAX_CHAR], int *n)
{
    (*n) = 0;
    char *pch;
    pch = strtok(command, " ");
    while (pch != NULL)
    {
        strcpy(tokens[(*n)++], pch);
        pch = strtok(NULL, " ");
    }
    return;
}

/*
FUNCTION : __sendfile
=====================
input : name (string), socket ID

+++++++++++++++++++++

[NOT USED JUST FOR TESTING PURPOSE]
Read the file and send the file in chunks and append '\0' for indicating the end of the file.

+++++++++++++++++++++

=====================

*/
void __sendfile(char *name,int sock_fd)
{
    int fd = open(name, O_RDONLY);
    if (fd == -1)
    {
        // perror("[ERROR] File can't be open\n");
        close(sock_fd);
        exit(1);
        return;
    }
    else
    {
        int byte_read = 0;
        do
        {
            char read_buffer[MAX_CHAR];
            byte_read = read(fd, read_buffer, MAX_CHAR - 1);
            read_buffer[byte_read] = '\0';
            send(sock_fd, read_buffer, strlen(read_buffer), 0);
            // printf("%s",read_buffer);
        } while (byte_read == MAX_CHAR - 1);
        send(sock_fd, "\0", strlen("\0") + 1, 0);
        close(sock_fd);
        close(fd);
        exit(0);
        return;
    }
}

/*
FUNCTION : __receivefile
=====================
input : name (string), socket ID

+++++++++++++++++++++

[NOT USED JUST FOR TESTING PURPOSE]
Receive data from tcp socket one byte at a time and end on receive '\0' extra. 

+++++++++++++++++++++

=====================

*/
void __receivefile(char* name,int sock_fd)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        // perror("[ERROR] Can't create a file\n");
        close(sock_fd);
        return;
    }
    else
    {
        char buf[2];
        do
        {
            recv(sock_fd, buf, 1, 0);
            buf[1] = '\0';
            if (buf[0] == '\0')
                break;
            write(fd, buf, 1);
        } while (buf[0] != '\0');
        close(sock_fd);
        close(fd);
        return;
    }
}

/*
FUNCTION : sendfile
=====================
input : name (string), socket ID

+++++++++++++++++++++

Read the file and send the file in chunk. Send a header before each chunk.
HEADER - < N <size of chunk> >  ( If not the last chunk), < L <size of chunk> > ( if the last chunk)

+++++++++++++++++++++

=====================

*/
void sendfile(char *name, int sock_fd)
{
    int fd = open(name, O_RDONLY);
    if (fd == -1)
    {
        // perror("[ERROR] File can't be open\n");
        close(sock_fd);
        exit(1);
        return;
    }
    else
    {
        int byte_read = 0;
        do
        {
            char read_buffer[MAX_CHAR];
            byte_read = read(fd, read_buffer, MAX_CHAR - 1);
            read_buffer[byte_read] = '\0';
            if (byte_read == MAX_CHAR - 1)
            {
                send(sock_fd, "N", 1, 0);
                send_int(byte_read+1,sock_fd);
            }
            else
            {
                send(sock_fd, "L", 1, 0);
                send_int(byte_read + 1, sock_fd);
            }
            
            send(sock_fd, read_buffer, strlen(read_buffer)+1, 0);
            // printf("%s",read_buffer);
        } while (byte_read == MAX_CHAR - 1);
        send(sock_fd, "\0", strlen("\0") + 1, 0);
        close(sock_fd);
        close(fd);
        exit(0);
        return;
    }
}

/*
FUNCTION : receivefile
=====================
input : name (string), socket ID

+++++++++++++++++++++

Receive the file with the help of heaer. First read the byte (N or L). Read the size of chunk and read that much amount of data.
HEADER - < N <size of chunk> >  ( If not the last chunk), < L <size of chunk> > ( if the last chunk)

+++++++++++++++++++++

=====================

*/
void receivefile(char *name, int sock_fd)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        // perror("[ERROR] Can't create a file\n");
        close(sock_fd);
        exit(1);
        return;
    }
    else
    {
        int total = 0;
        char buf[2];
        while(1)
        {
           int r =  recv(sock_fd, buf, 1, 0);
           if(r==0)
           {
               remove(name);
               break;
           }
            buf[1] = '\0';
            
            if(!strcmp(buf,"L"))
            {
                int num;
                get_int(sock_fd, &num);
                // printf("%d %s++\n", num, buf);
                char buffer[num];
                total+=num;
                recv(sock_fd, buffer, num, 0);
                write(fd, buffer, num-1);
                break;
            }
            else
            {
                int num;
                get_int(sock_fd, &num);
                // printf("%d %s\n",num,buf);
                char buffer[num];
                total += num;
                recv(sock_fd, buffer, num, 0);
                write(fd, buffer, num - 1);
            }
        }
        if(total==0)
        {
            remove(name);
            exit(1);
        }
        close(sock_fd);
        close(fd);
        exit(0);
        return;
    }
}
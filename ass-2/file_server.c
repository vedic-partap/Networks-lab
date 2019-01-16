/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 2

Server Side 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#define MAX_CHAR 100

int main()
{
    int sockfd,newsockfd; // craeting socket ids
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* create the socket socket(family, type, protocol) - 
    Socket call does not specify where data will be coming from,
    nor where it will be going to – it just creates the interface !*/

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[ERROR] Unable to create socket\n");
        exit(0);

    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8181);
    
    /* bind the socket
    bind(socket id, address , size)*/

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to bind the socket\n");
        close(sockfd);
        exit(0);
    }
    // make announcement for connection
    listen(sockfd, 5);
   
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);//accept the connection
    if(newsockfd<0)
    {
        perror("[ERROR] Unable to connect\n");
        close(sockfd);
        exit(0);
    }
    printf("[SUCCESS] Connected to Client \n");
    char filename[MAX_CHAR],temp_buf[MAX_CHAR];
    for(int i=0;i<MAX_CHAR;i++)
    {
        filename[i] = temp_buf [i]='\0';
    }
    int sz = 0,total_sz=0;
    total_sz = sz = recv(newsockfd, filename, MAX_CHAR,0);
    while ((sz = recv(newsockfd, temp_buf, MAX_CHAR, MSG_DONTWAIT)) > -1)
    {
        total_sz+=sz;
        strcat(filename,temp_buf);
    }
    int fd  = open(filename, O_RDONLY);
    if(fd==-1)
    {
        perror("[ERROR] File can't be open\n");
        close(newsockfd);
        close(sockfd);
        exit(0);
    }
    printf("[SUCCESS] File open - name %s\n",filename);
    int byte_read=0;
    do{
        char read_buffer[MAX_CHAR];
        byte_read = read(fd, read_buffer,MAX_CHAR-1);
        read_buffer[byte_read]='\0';
        send(newsockfd,read_buffer,strlen(read_buffer)+1,0);
        // printf("%s",read_buffer);
    }while(byte_read==MAX_CHAR-1);
    printf("[SUCCESS] Send\n");
    close(sockfd);
    close(newsockfd);
    close(fd);
    return 0;
    
}
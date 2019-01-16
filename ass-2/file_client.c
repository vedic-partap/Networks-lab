/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 2

Client Side 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define MAX_CHAR 100

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    // creating sockets
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // inet_aton("127.0.0,1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8181);
    // making connection to server
    if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to connect to server\n");
        close(sockfd);
        exit(0);
    }
    printf("[SUCCESS] Connected to Server \n");
    printf("Enter the filename : ");
    char filename[MAX_CHAR];
    scanf("%s",filename);

    // send the filename 
    send(sockfd, filename, strlen(filename)+1,0);
    printf("[SUCCESS] Filename send\n");
    // create the file
    int fd = open("client.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("[ERROR] Can't create a file\n");
        close(sockfd);
        exit(1);
    }
    // read the file data from the server
    int byte_rec=0,counter=0,nbytes=0,nwords=0,delim=1;
    char read_buf[MAX_CHAR];
    for (int i = 0; i < MAX_CHAR; i++)
        read_buf[i] = '\0';
    
    while ((byte_rec = recv(sockfd, read_buf, MAX_CHAR, 0))>0)
    {
        counter++;
        // writing data to the file
        write(fd,read_buf,strlen(read_buf));
        int j=0;
        while (j < byte_rec&&read_buf[j] != '\0')
        {
            nbytes++;
            // count the number of bytes and words
            if (read_buf[j] == ' ' || read_buf[j] == '\t' || 
            read_buf[j] == '\n' || read_buf[j] == ';' || read_buf[j] == ',' || 
            read_buf[j] == ':' || read_buf[j] == '.')
            delim=1;
            
            else
            {
                if(delim==1)
                {
                    nwords++;
                    delim=0;
                }
            }
            j++;
        }
        for (int i = 0; i < MAX_CHAR; i++) read_buf[i] = '\0';
    }
    // if file doesn't exist
    if (counter == 0 && byte_rec == 0)
    {
        printf("File Not Found\n");
        remove("client.txt");
        close(sockfd);
        close(fd);
        exit(0);
    }

    printf("[SUCESS] File received in %d calls\n",counter);
    printf("Number of words %d\nNumber of bytes %d\n",nwords,nbytes);
    close(sockfd);
    close(fd);
    return 0;
}
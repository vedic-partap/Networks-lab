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
#include <fcntl.h>

#define PORT 8182
#define MAX_CHAR 100

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    // creating sockets
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    // making connection to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("[ERROR] Unable to connect to server\n");
        close(sockfd);
        exit(0);
    }
    printf("[SUCCESS] Connected to Server \n");
    printf("Enter the filename : ");
    char filename[MAX_CHAR];
    scanf("%s", filename);

    // send the filename
    printf("%s\n",filename);
    send(sockfd, filename, strlen(filename) + 1, 0);
    printf("[SUCCESS] Filename send\n");
    // create the file
    // the file data from the server
    int counter = 0;
    char read_buf[2];
    for (int i = 0; i < 2; i++)
        read_buf[i] = '\0';
    // recv(sockfd, read_buf, 1, 0);
    // read_buf[1]='\0';
    int cntbz=0;
    int linelenght=0;
    while(cntbz<2)
    {
        recv(sockfd,read_buf,1,0);
        read_buf[1]='\0';
        if(read_buf[0]=='\0'){
            if(cntbz==1)
                break;
            if(linelenght>1)
            {
                counter++;
            }
            linelenght=0;
            cntbz=1;
        }
        if(read_buf[0]!='\0')
        {
            linelenght++;
            cntbz=0;
        }
        printf("%s",read_buf);

    }
    
    if (counter == 0)
    {
        printf("File Not Found\n");
        remove("client.txt");
        close(sockfd);
        exit(0);
    }
    printf("\nNumber of Words: %d\n",counter);
    printf("\n[SUCCESS] Completed\n");
    close(sockfd);
    return 0;
}
/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 1

Client Side 

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAX_CHARS 50
#define MAX_WORDS 100
#define MAX_LINE 1024

/*
Funtion to receive the string from the client
=========

Input:
sockfd : socket id 
buffer : character buffer for storing the received output
cliaddr: The address of the client

=========

 */
void receive(int sockfd, char buffer[], struct sockaddr_in *cliaddr)
{
    int n;
    socklen_t len;
    len = sizeof(*cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAX_LINE, 0,
                 (struct sockaddr *)cliaddr, &len);
    buffer[n] = '\0';
}
/*
Funtion to send the string to the client
=========

Input:
sockfd : socket id 
message : character message to be send to client
cliaddr: The address of the client

=========

 */
void sendmess(int sockfd, char message[], struct sockaddr_in *cliaddr)
{
    sendto(sockfd, (const char *)message, strlen(message), 0,
           (const struct sockaddr *)cliaddr, sizeof(*cliaddr));
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
    }
    //Initialization
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    char filename[MAX_CHARS];
    char buffer[MAX_LINE];

    printf("Enter the file name : ");
    scanf("%s",filename);
    sendmess(sockfd, filename, &servaddr);

    receive(sockfd, buffer, &servaddr);
    printf("SERVER: %s\n",buffer);
    
    if(!strcmp(buffer,"NOTFOUND"))
    {
        printf("ERROR: File not found !\n");
        close(sockfd);
    }
    else
    {
        FILE *fptr;
        int i=1;
        fptr = fopen("client.txt", "w");
        if(fptr==NULL)
        {
            printf("Sorry Client side file can't be made. Try Again\n");
        }
        else
        {
            while (i < MAX_WORDS && strcmp(buffer, "END"))
            {
                char word[MAX_CHARS] = "WORD";
                char temp[MAX_CHARS];
                sprintf(temp, "%d", i);
                strcat(word, temp);
                sendmess(sockfd, word, &servaddr);
                receive(sockfd, buffer, &servaddr);
                printf("SERVER: %s\n", buffer);
                if (!strcmp(buffer, "END")) break;
                fputs(buffer, fptr);
                fputs("\n", fptr);
                i++;
            }
            fclose(fptr);
            close(sockfd);
        }
    }
    return 0;
}

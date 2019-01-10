/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 1

Server Side 

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
typedef char string[MAX_CHARS + 1]; // leave one space for '\0'

/*
Funtion to receive the string from the client
=========

Input:
sockfd : socket id 
buffer : character buffer for storing the received output
cliaddr: The address of the client

=========

 */
void receive(int sockfd, char buffer[], struct sockaddr_in* cliaddr)
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
/*
Funtion to store all the words of a file in a array
=========

Input:
file: file name
array: the array in which the tokens will be stored

=========

 */
int tokenise(char file[],string array[])
{
    int cnt = 0, stop = 0;
    FILE *fptr;
    fptr = fopen(file, "r");
    if(fptr==NULL)
    {
        return 0;
    }
    
    else
    {
        for (int i = 0; (i < MAX_WORDS) && !stop; i++)
        {
            fscanf(fptr, "%s", array[i]);
            cnt++;
            if (!strcmp(array[i], "END"))
            {
                stop = 1;
                break;
            }
        }
        fclose(fptr);
        return cnt;
    }
    
    
}
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    // Initialization
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
    
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\nServer Running....\n");

    char buffer[MAX_LINE]; //Buffer to store the output
    string tokens[MAX_WORDS]; // Words token of the file
    receive(sockfd, buffer, &cliaddr);
    printf("CLIENT: %s\n", buffer);
    int filereceive = tokenise(buffer, tokens);
    if(filereceive==0)
    {
        char notfound[MAX_CHARS] = "NOTFOUND";
        sendmess(sockfd, notfound, &cliaddr);
        close(sockfd);
    }
    
    else
    {
        // send one by one words to client
        sendmess(sockfd, tokens[0], &cliaddr);
        for (int i = 1; i < filereceive; i++)
        {
            receive(sockfd, buffer, &cliaddr);
            printf("CLIENT: %s\n", buffer);
            sendmess(sockfd, tokens[i], &cliaddr);
        }
        close(sockfd);
    }
    
    
    return 0; 
}


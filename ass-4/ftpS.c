#include "myheader.h" // to incluse all the function and header file


// Function that will be called on PORT command

int PORT(char tokens[MAX_TOKENS][MAX_CHAR],int n,int sock_fd,int *port)
{
    int error ;
    if(n!=2)
        error = 503;
    else
    {
        *port = atoi(tokens[1]);
        if (*port <= 1024 || *port > 65535) // Check the conditons and dependin g on that decide the error code
            error = 503;
        else
            error = 200;
    }
    send_int(error,sock_fd);
    if((error==503))
        close(sock_fd);
    return error;
}

// Function called for cd command 
void CD(char tokens[MAX_TOKENS][MAX_CHAR], int n, int sock_fd)
{
    if (n != 2)
        send_int(501, sock_fd);
    else
    {
        int x = chdir(tokens[1]); // chnage the directory of on the server side
        if (x == -1)
            send_int(501, sock_fd);
        else
        {
            send_int(200, sock_fd);
            char s[100];
            printf("current directory : %s\n",getcwd(s,100));
        }
    }
    return ;    
} 

// Function called on get command
void GET(char tokens[MAX_TOKENS][MAX_CHAR],int n,int sockc_fd,int port)
{
    pid_t child;
    // For the process for SD
    if((child = fork())==0)
    {
        int sockd_fd;
        struct sockaddr_in clid_add;
        if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            // perror("[ERROR] Unable to create the socket\n");
            exit(1);
        }

        clid_add.sin_family = AF_INET;
        clid_add.sin_addr.s_addr = INADDR_ANY;
        clid_add.sin_port = htons(port);
        if (connect(sockd_fd, (struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
        {
            perror("[ERROR] Unable to connect to server\n");
            close(sockd_fd);
            exit(1);
        }
        sendfile(tokens[1],sockd_fd); // send the file asked by client (with header file)
    }
    else
    {
        int status;
        waitpid(child, &status, WUNTRACED); // wait for child process to send the file
        printf("%d\n",status);
        // Depending on the status of the return decide the error code
        if(status==0)
        {
            send_int(250,sockc_fd);
        }
        else
            send_int(550, sockc_fd);
    }
    return ;
}

// Function called on put command
void PUT(char tokens[MAX_TOKENS][MAX_CHAR], int n, int sockc_fd, int port)
{
    pid_t child;
    if ((child = fork()) == 0)
    {
        int sockd_fd;
        struct sockaddr_in clid_add;
        if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("[ERROR] Unable to create the socket\n");
            exit(1);
        }

        clid_add.sin_family = AF_INET;
        clid_add.sin_addr.s_addr = INADDR_ANY;
        clid_add.sin_port = htons(port);
        if (connect(sockd_fd, (struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
        {
            perror("[ERROR] Unable to connect to server\n");
            close(sockd_fd);
            exit(1);
        }
        receivefile(tokens[1], sockd_fd);
        exit(0);
    }
    else
    {
        int status;
        waitpid(child, &status, WUNTRACED);
        if (status == 0)
        {
            send_int(250, sockc_fd);
        }
        else
            send_int(550, sockc_fd);
    }
    return;
}

int main()
{
    int sockc_fd,clientc_fd,newsockc_fd;
    socklen_t clic_len;
    struct sockaddr_in servc_add;
    
    if((sockc_fd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(1);
    }

    memset(&servc_add,0,sizeof(servc_add));
    servc_add.sin_family = AF_INET;
    servc_add.sin_addr.s_addr = INADDR_ANY;
    servc_add.sin_port = htons(PORTX);

    //Bind the socket 
    if(bind(sockc_fd,(const struct sockaddr *)&servc_add,sizeof(servc_add))<0)
    {
        perror("[ERROR] Unable to bind the socket X\n");
        exit(1);
    }

    //Listen 
    listen(sockc_fd,5);
    while(1)
    {
        int count = 0;
        printf("Waiting for the Client to connect...\n");
        clic_len = sizeof(clientc_fd);
        newsockc_fd = accept(sockc_fd,(struct sockaddr *)&clientc_fd,&clic_len);
        printf("Client Control is Connected\n");
        while(1)
        {
            char buf[2], buffer[MAX_CHAR];
            int idx = 0, port;
            do
            {
                recv(newsockc_fd, buf, 1, 0);
                buf[1] = '\0';
                buffer[idx++] = buf[0];
            } while (buf[0] != '\0');
            printf("> %s\n", buffer);
            count++;
            char tokens[MAX_TOKENS][MAX_CHAR];
            int n;
            tokenise(buffer, tokens, &n);

            if ((count == 1 && strcmp(tokens[0], "PORT"))||n>2) // check i fthe port is first command or not
            {
                send_int(503, newsockc_fd);
                close(newsockc_fd);
                break;
            }
            if (n > 0 && (!strcmp(tokens[0], "PORT") ||
                          !strcmp(tokens[0], "cd") || !strcmp(tokens[0], "get") ||
                          !strcmp(tokens[0], "put") || !strcmp(tokens[0], "quit")))
            {
                if (n > 2) // check the validity of the command
                {
                    send_int(501, newsockc_fd);
                }
                else if (n > 0 && !strcmp(tokens[0], "PORT"))
                {
                    int error = PORT(tokens, n, newsockc_fd, &port);
                    if (error == 503 || error == 550)
                        break;
                }
                else if (n > 0 && !strcmp(tokens[0], "cd"))
                {
                    CD(tokens, n, newsockc_fd);
                }
                else if (n > 0 && !strcmp(tokens[0], "get"))
                {
                    GET(tokens, n, newsockc_fd, port);
                }
                else if (n > 0 && !strcmp(tokens[0], "put"))
                {
                    PUT(tokens, n, newsockc_fd, port);
                }
                else if (n > 0 && !strcmp(tokens[0], "quit"))
                {
                    send_int(421, newsockc_fd);
                    close(newsockc_fd);
                    break;
                }
            }
            else
            {
                send_int(502,newsockc_fd); // when command is not from any of the above
            }
            
        }
    }

}
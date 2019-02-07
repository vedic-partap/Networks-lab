#include "myheader.h"

void error(int x,int code,int sock_fd)
{
    if(x==502)
    {
        printf(": Invalid command\n");
    }
    else if(x==501)
    {
        printf(": Please enter commnd with correct input\n");
    }
    else if (x == 503)
    {
        printf(": Enter port command first and correctly\n");
        close(sock_fd);
        exit(0);
    }
    else if(code == 0)
    {
        if(x==503)
        {
            printf(": Enter port command first and correctly\n");
            close(sock_fd);
            exit(0);
        }
        else if (x == 550)
        {
            printf(": Unable to send command\n");
            close(sock_fd);
            exit(0);
        }
        else if(x == 200)
        {
            printf(": Success\n");
        }
    }
    else if(code == 1)
    {
        if (x == 501)
        {
            printf(": Unable to execute\n");
        }
        else if (x == 200)
        {
            printf(": Success\n");
        }
    }
    else if (code == 2||code == 3)
    {
        if (x == 550)
        {
            printf(": File can't be found or unable to read/write\n");
        }
        else if (x == 250)
        {
            printf(": Success\n");
        }
    }
    else
    {
        printf(": Exit\n");
        exit(0);
    }
    
}
void PORT(char* command,char tokens[MAX_TOKENS][MAX_CHAR],int n,int sock_fd)
{
    send(sock_fd, command, strlen(command) + 1, 0);
}

void GET(char* command, char tokens[MAX_TOKENS][MAX_CHAR],int n, int sockc_fd,int port)
{
    pid_t clinet_d;
    if((clinet_d=fork())==0)
    {
        int sockd_fd, newsockd_fd;
        socklen_t servd_len;
        struct sockaddr_in clid_add,serv_add;

        if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("[ERROR] Unable to create the socket\n");
            exit(0);
        }

        memset(&clid_add, 0, sizeof(clid_add));
        memset(&serv_add,0,sizeof(serv_add));
        clid_add.sin_family = AF_INET;
        clid_add.sin_addr.s_addr = INADDR_ANY;
        clid_add.sin_port = htons(port);

        //Bind the socket
        if (bind(sockd_fd, (const struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
        {
            perror("[ERROR] Unable to bind the socket Y\n");
            exit(0);
        }
        //Listen
        // printf("Port : %d\n", port);
        // printf("Listening ...\n");
        listen(sockd_fd, 5);
        servd_len = sizeof(serv_add);
        newsockd_fd = accept(sockd_fd, (struct sockaddr *)&serv_add, &servd_len);
        receivefile(tokens[1],newsockd_fd);
        close(sockd_fd);
        exit(0);
    }
    else
    {
        send(sockc_fd, command, strlen(command) + 1, 0);
        int status;
        waitpid(clinet_d, &status, 0);
    }
}

void PUT(char *command, char tokens[MAX_TOKENS][MAX_CHAR], int n, int sockc_fd, int port)
{
    {
        pid_t clinet_d;
        if ((clinet_d = fork()) == 0)
        {
            int sockd_fd, newsockd_fd;
            socklen_t servd_len;
            struct sockaddr_in clid_add, serv_add;

            if ((sockd_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("[ERROR] Unable to create the socket\n");
                exit(0);
            }

            memset(&clid_add, 0, sizeof(clid_add));
            memset(&serv_add, 0, sizeof(serv_add));
            clid_add.sin_family = AF_INET;
            clid_add.sin_addr.s_addr = INADDR_ANY;
            clid_add.sin_port = htons(port);

            //Bind the socket
            if (bind(sockd_fd, (const struct sockaddr *)&clid_add, sizeof(clid_add)) < 0)
            {
                perror("[ERROR] Unable to bind the socket Y\n");
                exit(0);
            }
            //Listen
            // printf("Port : %d\n", port);
            // printf("Listening ...\n");
            listen(sockd_fd, 5);
            servd_len = sizeof(serv_add);
            newsockd_fd = accept(sockd_fd, (struct sockaddr *)&serv_add, &servd_len);
            sendfile(tokens[1], newsockd_fd);
            close(sockd_fd);
            exit(0);
        }
        else
        {
            send(sockc_fd, command, strlen(command) + 1, 0);
            int status;
            waitpid(clinet_d, &status, 0);
        }
    }
    
    return;
}

int main()
{
    int sockc_fd;
    struct sockaddr_in servc_add;
    char command[MAX_CHAR];
    if ((sockc_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[ERROR] Unable to create the socket\n");
        exit(0);
    }

    servc_add.sin_family = AF_INET;
    servc_add.sin_addr.s_addr = INADDR_ANY;
    servc_add.sin_port = htons(PORTX);
    // making connection to server
    if (connect(sockc_fd, (struct sockaddr *)&servc_add, sizeof(servc_add)) < 0)
    {
        perror("[ERROR] Unable to connect to server\n");
        close(sockc_fd);
        exit(0);
    }
    int port = 1024;
    while(1)
    {
        printf("> ");
        scanf("%[^\n]%*c", command);

        char temp[MAX_CHAR],tokens[MAX_TOKENS][MAX_CHAR];
        int n,code;
        strcpy(temp,command);
        tokenise(temp,tokens,&n);
        
        if(!strcmp(tokens[0],"PORT"))
        {
            port = atoi(tokens[1]);
            PORT(command,tokens,n,sockc_fd);
            code =0;
        }
        else if(!strcmp(tokens[0],"get"))
        {
            code = 2;
            if(port != 1024)
            {
                GET(command, tokens, n, sockc_fd, port);
            }
            else
            {
                send(sockc_fd, command, strlen(command) + 1, 0);
            }
            
        }
        else if (!strcmp(tokens[0], "put"))
        {
            code = 3;
            if(port!=1024)
            {
                PUT(command, tokens, n, sockc_fd, port);
            }
            else
            {
                send(sockc_fd, command, strlen(command) + 1, 0);
            }
            
        }
        else
        {
            send(sockc_fd, command, strlen(command) + 1, 0);
            code = (!strcmp(tokens[0], "cd") ? 1: 4);
        }

        int error_code;
        sleep(0.01);
        get_int(sockc_fd,&error_code);
        printf("%d",error_code);
        error(error_code, code, sockc_fd);
        fflush(STDIN_FILENO);

    }
    return 0;
}
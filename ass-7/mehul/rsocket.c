// headers
#include "rsocket.h"

//GLobal VAriables for socket ---START------------------------------------------------------/
int udp_fd = -1;
int buffer_filled = 0;
int counter = 0;
struct sockaddr recv_source_addr;
char receive_buffer[BUF_SIZE];
socklen_t recv_addr_len;
int recv_flags;

pthread_t tid;

struct receive_message_id
{
    int id;
    struct sockaddr src_addr;
} * receive_message_id_table;

typedef struct unacknowledged_message
{
    int id;
    char msg[MSG_SIZE];
    size_t msg_len;
    time_t time;
    int flags;
    struct sockaddr dest_addr;
    socklen_t addrlen;
} UnACK;
UnACK *unacknowledged_message_table;
//GLobal VAriables for socket ---END------------------------------------------------------/

// Auxilary functions not accesible by the user(hence, not in rsocket.h)
int Increment();
int GetEmptyPlace_recvid();
size_t combineIntString(int id, char *buf, int len);
void Decompose(int *id, char *buf, int len);
int DEL_from_unacknowledged_message_table(int id);
UnACK *GetEmptyPlace_unack();
// int  ADD_to_unacknowledged_message_table(UnACK unack_msg);

ssize_t send_ACK(int id, struct sockaddr addr, socklen_t addr_len)
{
    char ACK[BUF_SIZE];
    strcpy(ACK, "ACK");
    combineIntString(id, ACK, -1);
    int r = sendto(udp_fd, ACK, strlen(ACK) + sizeof(id), 0, &addr, addr_len);
    // printf("ACK id: %d err: %d\n",id,r );
}

int HandleACKMsgReceive(int id)
{
    printf("ACK %d\n", id);
    return DEL_from_unacknowledged_message_table(id);
}

int HandleAppMsgReceive(int id, char *buf, struct sockaddr source_addr, socklen_t addr_len)
{
    int present = 0;

    for (int i = 0; i < TABLE_SIZE; i++)
        if (receive_message_id_table[i].id == id)
            // && receive_message_id_table[i].src_addr. = source_addr)
            present = 1;

    if (!present)
    {
        strcpy(receive_buffer, buf);
        recv_source_addr = source_addr;
        recv_addr_len = addr_len;
        buffer_filled = strlen(receive_buffer);
        int i = GetEmptyPlace_recvid();
        if (i < 0)
            return -1;
        receive_message_id_table[i].id = id;
        receive_message_id_table[i].src_addr = source_addr;
    }
    send_ACK(id, source_addr, addr_len);
    return 0;
}

int HandleReceive()
{

    char buf[BUF_SIZE];
    memset(buf, '\0', sizeof buf);

    struct sockaddr source_addr;

    socklen_t addr_len = sizeof(source_addr);
    int n = recvfrom(udp_fd, buf, BUF_SIZE, recv_flags, (struct sockaddr *)&source_addr, &addr_len);

    if (dropMessage(DROP_PROBALITY))
        return 0;

    int id;
    Decompose(&id, buf, n);
    // printf("HandleReceive: %s %d\n", buf,id);
    printf("Receive: %s id: %d\n", buf, id);

    if (strcmp(buf, "ACK") == 0)
        return HandleACKMsgReceive(id);
    else
        return HandleAppMsgReceive(id, buf, source_addr, addr_len);
}

int HandleRetransmit()
{
    time_t time_now = time(NULL);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unacknowledged_message_table[i].id != -1 &&
            unacknowledged_message_table[i].time + TIMEOUT <= time_now)
        {
            ssize_t r = sendto(udp_fd, unacknowledged_message_table[i].msg,
                               unacknowledged_message_table[i].msg_len,
                               unacknowledged_message_table[i].flags,
                               &unacknowledged_message_table[i].dest_addr,
                               unacknowledged_message_table[i].addrlen);
            unacknowledged_message_table[i].time = time_now;
            printf("Retransmit %d\n", unacknowledged_message_table[i].id);
            if (r < 0)
                return -1;
        }
    }
    return 0;
}

void *threadX(void *param)
{
    // printf("threadX\n");
    fd_set rfds;
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(udp_fd, &rfds);

        int r = select(udp_fd + 1, &rfds, NULL, NULL, &timeout);
        if (r < 0)
        {
            perror("Select Failed\n");
        }
        else if(r)
        {
            if (FD_ISSET(udp_fd, &rfds))
            { //came out when received a message
                // printf("udp_fd\n");
                HandleReceive();
            }
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
            // printf("TIMEOUT\n");
            HandleRetransmit();
        }
        
        // if (!FD_ISSET(udp_fd, &rfds))
        // { //came out by timeout
        //     timeout.tv_sec = TIMEOUT;
        //     // printf("TIMEOUT\n");
        //     HandleRetransmit();
        // }
        // else if (FD_ISSET(udp_fd, &rfds))
        // { //came out when received a message
        //     // printf("udp_fd\n");
        //     HandleReceive();
        // }
    }
}

int r_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MRP)
        return -1;

    if((udp_fd = socket(domain, SOCK_DGRAM, protocol))<0)
        return udp_fd;

    //init tables
    unacknowledged_message_table = (UnACK *)
        malloc(TABLE_SIZE * sizeof(UnACK));
    receive_message_id_table = (struct receive_message_id *)
        malloc(TABLE_SIZE * sizeof(struct receive_message_id));

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        receive_message_id_table[i].id = -1;
        unacknowledged_message_table[i].id = -1; // -1 means id not presesnt
    }

    //Thread X creation
    pthread_attr_t attr; //Set of thread attributes
    pthread_attr_init(&attr);
    int ret = pthread_create(&tid, &attr, threadX, NULL);
    if(ret<0)
        return -1;

    return udp_fd;
}

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    return bind(socket, address, address_len);
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{

    if (sockfd != udp_fd)
        return -1;

    int cnt = Increment();

    char *buff = (char *)buf;

    // ADD to unacknowledged message table
    UnACK *unack_msg = GetEmptyPlace_unack();
    if (unack_msg == NULL)
        return -1;

    unack_msg->id = cnt;
    unack_msg->time = time(NULL);

    // unack_msg->msg = (char*)malloc(len + sizeof(unack_msg->id));

    strcpy(unack_msg->msg, buff);
    size_t byte_final = combineIntString(unack_msg->id, unack_msg->msg, len);

    assert(byte_final==len + sizeof(unack_msg->id));
    unack_msg->msg_len = byte_final;
    unack_msg->flags = flags;
    unack_msg->dest_addr = *dest_addr;
    unack_msg->addrlen = addrlen;

    ssize_t r = sendto(sockfd, unack_msg->msg,
                       unack_msg->msg_len,
                       unack_msg->flags,
                       &unack_msg->dest_addr,
                       unack_msg->addrlen);

    // printf("Send : %s id: %d\n", unack_msg->msg, cnt);
    return r;
}

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    if (sockfd != udp_fd)
        return -1;

    while (1)
    {
        if (buffer_filled)
        {
            for (int i = 0; i < buffer_filled; i++)
                buf[i] = receive_buffer[i];
            len = buffer_filled;
            *src_addr = recv_source_addr;
            *addrlen = recv_addr_len;
            recv_flags = flags;
            buffer_filled = 0;
            return len;
        }
        else if (flags == MSG_DONTWAIT)
            break;
        else
            sleep(0.001);
    }
}

int r_close(int sockfd)
{
    if (sockfd != udp_fd)
        return -1;

    while (1)
    {
        int flag = 0;
        for (int i = 0; i < TABLE_SIZE; i++)
            if (unacknowledged_message_table[i].id != -1)
                flag = 1;
        if (!flag)
            break;
    }
    pthread_kill(tid, SIGKILL);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        free(unacknowledged_message_table[i].msg);
    }
    free(unacknowledged_message_table);
    free(receive_message_id_table);
    return close(sockfd);
}

int dropMessage(float p)
{
    int r = random() % 10;
    return (r < p * 10);
}

/*-------------------------------------------------------------------------------*/
// Auxilary functions
int Increment()
{
    return ++counter;
}

UnACK *GetEmptyPlace_unack()
{

    for (int i = 0; i < TABLE_SIZE; i++)
        if (unacknowledged_message_table[i].id == -1)
            return &unacknowledged_message_table[i];
    return NULL;
}

int GetEmptyPlace_recvid()
{
    for (int i = 0; i < TABLE_SIZE; i++)
        if (receive_message_id_table[i].id == -1)
            return i;
    return -1;
}

// int ADD_to_unacknowledged_message_table(UnACK unack_msg){
//     int i = GetEmptyPlace();
//     if(i < 0)
//         return -1;

//     unacknowledged_message_table[i] = unack_msg;
//     return 0;
// }

int DEL_from_unacknowledged_message_table(int id)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unacknowledged_message_table[i].id == id)
        {
            unacknowledged_message_table[i].id = -1;
            // free(unacknowledged_message_table[i].msg);
            return 0;
        }
    }
    return -1;
}

size_t combineIntString(int id, char *buf, int len)
{
    if (len == -1)
        len = strlen(buf);
    for (size_t i = len; i < len + sizeof(id); i++)
        buf[i] = '\0';
    // printf("Checking :%s %d\n", (char *)&id, id);
    strcat(buf + len + 1, (char *)&id);
    return len+sizeof(id);
}
void Decompose(int *id, char *buf, int len)
{
    int *ret;
    len = strlen(buf);
    ret = (int *)(buf + len + 1);
    *id = *ret;
}
/*-------------------------------------------------------------------------------*/

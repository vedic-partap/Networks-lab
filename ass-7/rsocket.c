#include "rsocket.h"

//GLobal VAriables for socket ---START------------------------------------------------------/
int udp_fd = -1;
int counter = 0;
struct sockaddr_in recv_source_addr;
socklen_t recv_addr_len = 0;
int recv_flags = 0;
int start_rb = 0, end_rb = 0, buffer_count = 0;
int cnt_trans = 0;
pthread_t tid;
pthread_mutex_t lock;

/*Data Structures */
struct receive_buffer_entry
{
    char buffer[MSG_SIZE];
    struct sockaddr_in recv_addr;
} * receive_buffer;

struct receive_message_id
{
    int id;
    struct sockaddr_in src_addr;
    socklen_t addrlen;
} * receive_message_id_table;

typedef struct unacknowledged_message
{
    int id;
    char msg[MSG_SIZE];
    size_t msg_len;
    time_t time;
    int flags;
    struct sockaddr_in dest_addr;
    socklen_t addrlen;
} unAckTable;
unAckTable *unacknowledged_message_table;
//GLobal VAriables for socket ---END------------------------------------------------------/

// Auxilary functions not accesible by the user(hence, not in rsocket.h)
int Increment();
int HandleACKMsgReceive(int id);
int HandleAppMsgReceive(int id, char *buf, struct sockaddr_in source_addr, socklen_t addr_len) ;
int getEmptyPlaceRecvid();
size_t combineIntString(int id, char *buf, int len);
void breakIntString(int *id, char *buf, int len);
int delFromUnackTable(int id);
int HandleReceive();
int HandleRetransmit();
unAckTable *GetEmptyPlace_unack();
// int  ADD_to_unacknowledged_message_table(unAckTable unack_msg);
// function to creat eth socket
int r_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MRP)
        return -1;

    if ((udp_fd = socket(domain, SOCK_DGRAM, protocol)) < 0)
        return udp_fd;

    //init tables
    unacknowledged_message_table = (unAckTable *)
        malloc(TABLE_SIZE * sizeof(unAckTable));

    receive_message_id_table = (struct receive_message_id *)
        malloc(TABLE_SIZE * sizeof(struct receive_message_id));

    receive_buffer = (struct receive_buffer_entry *)
        malloc(BUFFER_SIZE * sizeof(struct receive_buffer_entry));

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        receive_message_id_table[i].id = -1;
        unacknowledged_message_table[i].id = -1; // -1 means id not presesnt
    }
    pthread_mutex_init(&lock,NULL);
    start_rb = end_rb = buffer_count = 0;
    //Thread X creation
    pthread_attr_t attr; //Set of thread attributes
    pthread_attr_init(&attr);
    int ret = pthread_create(&tid, &attr, runnerX, NULL);
    if (ret < 0)
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
    unAckTable *unack_msg = GetEmptyPlace_unack();
    if (unack_msg == NULL)
        return -1;

    unack_msg->id = cnt;
    unack_msg->time = time(NULL);
    strcpy(unack_msg->msg, buff);
    size_t byte_final = combineIntString(unack_msg->id, unack_msg->msg, len);

    assert(byte_final == len + sizeof(unack_msg->id));
    unack_msg->msg_len = byte_final;
    unack_msg->flags = flags;
    unack_msg->dest_addr = *(struct sockaddr_in *)dest_addr;
    unack_msg->addrlen = addrlen;

    ssize_t r = sendto(sockfd, unack_msg->msg,
                       unack_msg->msg_len,
                       unack_msg->flags,
                       (struct sockaddr *)&unack_msg->dest_addr,
                       unack_msg->addrlen);
    cnt_trans++;
    return r;
}

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    if (sockfd != udp_fd)
        return -1;
    // printf("yes\n");
    while (1)
    {
        if (buffer_count > 0)
        {
            strcpy(buf, receive_buffer[start_rb].buffer);
            buffer_count--;
            receive_buffer[start_rb].recv_addr = recv_source_addr;
            start_rb = (start_rb + 1) % BUFFER_SIZE;
            if (len >= 0 && len < strlen(buf))
            {
                buf[len] = '\0';
            }
            len = strlen(buf);
            *src_addr = *(struct sockaddr *)&recv_source_addr;
            *addrlen = recv_addr_len;
            recv_flags = flags;
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
    printf("No of transmissions = %d\n", cnt_trans);
    pthread_kill(tid, SIGKILL);
    free(receive_buffer);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        free(unacknowledged_message_table[i].msg);
    }
    free(unacknowledged_message_table);
    free(receive_message_id_table);

    return close(sockfd);
}

ssize_t sendACK(int id, struct sockaddr_in addr, socklen_t addr_len)
{
    char ACK[BUFFER_SIZE];
    memset(ACK, '\0', sizeof(ACK));
    strcpy(ACK, "ACK");
    size_t ret = combineIntString(id, ACK, -1);
    size_t r = sendto(udp_fd, ACK, ret, 0, (struct sockaddr *)&addr, addr_len);
    return r;
}

void *runnerX(void *param)
{
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
        else if (r)
        {
            if (FD_ISSET(udp_fd, &rfds))
            { //came out when received a message
                HandleReceive();
            }
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
            HandleRetransmit();
        }
    }
}

int HandleACKMsgReceive(int id)
{
    printf("ACK %d\n", id);
    return delFromUnackTable(id);
}

int HandleAppMsgReceive(int id, char *buf, struct sockaddr_in source_addr, socklen_t addr_len)
{
    int present = 0;

    for (int i = 0; i < TABLE_SIZE; i++)
        if (receive_message_id_table[i].id == id)
            present = 1;

    if (!present)
    {
        strcpy(receive_buffer[end_rb].buffer, buf);
        recv_source_addr = source_addr;
        recv_addr_len = addr_len;
        buffer_count++;
        end_rb = (end_rb + 1) % BUFFER_SIZE;
        int i = getEmptyPlaceRecvid();
        if (i < 0)
            return -1;
        receive_message_id_table[i].id = id;
        receive_message_id_table[i].src_addr = source_addr;
        receive_message_id_table[i].addrlen = addr_len;
    }
    sendACK(id, source_addr, addr_len);
    return 0;
}

int HandleReceive()
{

    char buf[BUFFER_SIZE];
    memset(buf, '\0', sizeof buf);

    struct sockaddr_in source_addr;

    socklen_t addr_len = sizeof(source_addr);
    int n = recvfrom(udp_fd, buf, BUFFER_SIZE, recv_flags, (struct sockaddr *)&source_addr, &addr_len);
    buf[n] = '\0';
    if (dropMessage(DROP_PROBALITY))
        return 0;

    int id;
    breakIntString(&id, buf, n);
    if (!strcmp(buf, "ACK"))
        return HandleACKMsgReceive(id);
    else
        return HandleAppMsgReceive(id, buf, source_addr, addr_len);
}

int HandleRetransmit()
{
    time_t time_now = time(NULL);
    // pthread_mutex_lock(&lock);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unacknowledged_message_table[i].id != -1 &&
            unacknowledged_message_table[i].time + TIMEOUT <= time_now)
        {
            ssize_t r = sendto(udp_fd, unacknowledged_message_table[i].msg,
                               unacknowledged_message_table[i].msg_len,
                               unacknowledged_message_table[i].flags,
                               (struct sockaddr *)&unacknowledged_message_table[i].dest_addr,
                               unacknowledged_message_table[i].addrlen);
            unacknowledged_message_table[i].time = time_now;
            // pthread_mutex_unlock(&lock);
            printf("Retransmit %d\n", unacknowledged_message_table[i].id);
            cnt_trans++;
            if (r < 0)
                return -1;
        }
    }
    return 0;
}

int dropMessage(float p)
{
    float r = (float)rand() / ((float)RAND_MAX);
    // printf("r = %f, p = %f\n",r,p);
    return (r < p);
}

/*-------------------------------------------------------------------------------*/
// Auxilary functions for help
int Increment()
{
    return ++counter;
}

unAckTable *GetEmptyPlace_unack()
{

    for (int i = 0; i < TABLE_SIZE; i++)
        if (unacknowledged_message_table[i].id == -1)
            return &unacknowledged_message_table[i];
    return NULL;
}

int getEmptyPlaceRecvid()
{
    for (int i = 0; i < TABLE_SIZE; i++)
        if (receive_message_id_table[i].id == -1)
            return i;
    return -1;
}

int delFromUnackTable(int id)
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
    strcat(buf + len + 1, (char *)&id);
    return len + sizeof(id);
}
void breakIntString(int *id, char *buf, int len)
{
    int *ret;
    len = strlen(buf);
    ret = (int *)(buf + len + 1);
    *id = *ret;
}

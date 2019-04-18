/* Vedic Partap
16CS10053 */


/* How to RUN:
$] gcc mytraceroute_16CS10053.c -o tr
$] sudo ./tr www.iitkgp.ac.in */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define N 52
#define MSG_SIZE 2048
#define MAX_CHAR 100
#define PCKT_LEN 8192
#define TIMEOUT 1
#define MAX_HOP 64
#define DEST_PORT 32164
#define S_PORT 8080

/* Function to generate random string */
void gen(char *dst)
{
    for (int i = 0; i < N; i++)
    {
        dst[i] = rand() % 26 + 'A';
    }
    dst[N-1] = '\0';
}

/*The IPv4 layer generates an IP header when sending a packet unless the IP_HDRINCL socket option is enabled on the socket. 
When it is enabled, the packet must contain an IP header.*/

/* Function to find IP  */
int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] == NULL)
        return 1;
    else
    {
        strcpy(ip, inet_ntoa(*addr_list[0]));
        return 0;
    }
}
/* Check sum function  */
unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[])
{
    int rawfd1, rawfd2;
    struct sockaddr_in saddr_raw, cli_addr;
    socklen_t saddr_raw_len;

    /* 1. Create two Raw Socket */
    if ((rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
    {
        perror("Socket error");
        exit(1);
    }
    if ((rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("Socket error");
        exit(1);
    }
    if (argc != 2)
    {
        printf("Error: Invalid parameters!\n");
        printf("Usage: %s <target hostname/IP>\n", argv[0]);
        close(rawfd1);
        close(rawfd2);
        exit(1);
    }
    /* 2. get the destination IP */
    char ipaddr[MAX_CHAR];
    if (hostname_to_ip(argv[1], ipaddr) != 0)
    {
        close(rawfd1);
        close(rawfd2);
        exit(1);
    }

    u_int16_t src_port, dst_port;
    u_int32_t dst_addr;
    dst_addr = inet_addr(ipaddr);
    src_port = S_PORT;
    dst_port = DEST_PORT;
    saddr_raw.sin_family = AF_INET;
    saddr_raw.sin_port = htons(src_port);
    saddr_raw.sin_addr.s_addr = INADDR_ANY; //inet_addr(LISTEN_IP);
    saddr_raw_len = sizeof(saddr_raw);
    /* 3. Bind the Sockets */
    if (bind(rawfd1, (struct sockaddr *)&saddr_raw, saddr_raw_len) < 0)
    {
        perror("raw bind");
        close(rawfd1);
        close(rawfd2);
        exit(1);
    }

    printf("mytraceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], ipaddr, MAX_HOP, N);

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(dst_port);
    cli_addr.sin_addr.s_addr = dst_addr;

    int one = 1;
    const int *val = &one;
    if (setsockopt(rawfd1, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        fprintf(stderr, "Error: setsockopt. You need to run this program as root\n");
        close(rawfd1);
        close(rawfd2);
        exit(1);
    }
    int ttl = 1, timeout = TIMEOUT, is_send = 1;
    fd_set readSockSet;
    int times = 0;
    char payload[52];
    clock_t start_time;
    while (1)
    {
        if (ttl >= 64)
            break;
        char buffer[PCKT_LEN];
        struct iphdr *ip = (struct iphdr *)buffer;
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
        if (is_send)
        {
            /* 4. generate Payload */
            times++;
            gen(payload);
            memset(buffer, 0, PCKT_LEN);

            /* 5. Generate UPD and IP header */
            ip->ihl = 5;
            ip->version = 4;
            ip->tos = 0; // low delay
            ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + N; //https://tools.ietf.org/html/rfc791#page-11
            ip->id = htons(54322);
            ip->ttl = ttl;     // hops
            ip->protocol = 17; // UDP
            ip->saddr = 0;     //src_addr;
            ip->daddr = dst_addr;

            // fabricate the UDP header
            udp->source = htons(src_port);
            // destination port number
            udp->dest = htons(dst_port);
            udp->len = htons(sizeof(struct udphdr)+N);

            // calculate the checksum for integrity
            ip->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));

            /* 6. Send the packet */
            strcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), payload);
            if (sendto(rawfd1, buffer, ip->tot_len, 0,
                       (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
            {
                perror("sendto()");
                close(rawfd1);
                close(rawfd2);
                exit(1);
            }
            // printf("packet Send %d\n", ttl);
            start_time = clock();

        }
        /* 7. Wait on select call */
        FD_ZERO(&readSockSet);
        FD_SET(rawfd2, &readSockSet);
        struct timeval tv = {timeout, 0};
        int ret = select(rawfd2 + 1, &readSockSet, 0, 0, &tv);
        if (ret == -1)
        {
            perror("select()\n");
            close(rawfd1);
            close(rawfd2);
            exit(1);
        }
        else if (ret)
        {
            // ICMP
            if (FD_ISSET(rawfd2, &readSockSet))
            {
                /* 8. Read the ICMP Message */
                // printf("ICMP\n");
                char msg[MAX_CHAR];
                int msglen;
                socklen_t raddr_len = sizeof(saddr_raw);
                msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *)&saddr_raw, &raddr_len);
                clock_t end_time = clock();
                if (msglen <= 0)
                {
                    timeout = TIMEOUT;
                    is_send = 1;
                    continue;
                }
                struct iphdr hdrip = *((struct iphdr *)msg);
                int iphdrlen = sizeof(hdrip);
                struct icmphdr hdricmp = *((struct icmphdr *)(msg + iphdrlen));
                /* 9. Handle Different Case */
                // read the destination IP
                struct in_addr saddr_ip;
                saddr_ip.s_addr = hdrip.saddr;
                if (hdrip.protocol == 1) //ICMP
                {
                    if (hdricmp.type == 3)
                    {
                        // verify
                        if (hdrip.saddr == ip->daddr)
                        {
                            printf("%d\t%s\t%.3f ms\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000);
                        }
                        close(rawfd1);
                        close(rawfd2);
                        return 0;
                    }
                    else if (hdricmp.type == 11)
                    {
                        //time exceed
                        printf("%d\t%s\t%.3f ms\n", ttl, inet_ntoa(saddr_ip), (float)(end_time - start_time) / CLOCKS_PER_SEC * 1000);
                        ttl++;
                        times = 1;
                        timeout = TIMEOUT;
                        is_send = 1;
                        continue;
                    }
                }
                else
                {
                    //Ignore the message
                    // printf("ignore\n");
                    is_send = 0;
                    timeout = end_time - start_time;
                    if (timeout >= 0.01)
                        continue;
                    else
                    {
                        if (times > 3)
                        {
                            printf("%d\t*\t*\n", ttl);
                            times = 1;
                            ttl++;
                        }
                        timeout = TIMEOUT;
                        is_send = 1;
                        continue;
                    }
                }
            }
        }
        else
        {
            //timeout
            // printf("Timeout\n");
            if (times > 3)
            {
                printf("%d\t*\t*\n", ttl);
                times = 1;
                ttl++;
            }
            timeout = TIMEOUT;
            is_send = 1;
            continue;
        }
    }
    close(rawfd1);
    close(rawfd2);
    return 0;
}

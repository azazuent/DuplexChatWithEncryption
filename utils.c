#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE 128

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void wait_send(int *fd)
{
    char sendbuf[MAX_DATA_SIZE];
    while(1)
    {
        memset(sendbuf, 0, MAX_DATA_SIZE);
        fgets(sendbuf, MAX_DATA_SIZE, stdin);

        int msg_length = strlen(sendbuf);
        sendbuf[msg_length] = '\0';

        if (send(*fd, sendbuf, msg_length+1, 0) < 0)
        {
            printf("Send failed\n");
            exit(1);
        }
    }
}

void wait_recv(int *fd)
{
    char recvbuf[MAX_DATA_SIZE];
    int msg_length;
    while(1)
    {
        memset(recvbuf, 0, MAX_DATA_SIZE);
        if (msg_length = recv(*fd, recvbuf, MAX_DATA_SIZE, 0) == -1)
        {
            perror("recv");
            exit(1);
        }
        if (recvbuf[0] == '\0')
        {
            printf("Client disconnected, exiting...\n");
            exit(0);
        }
        printf("Received: %s", recvbuf);
    }
}
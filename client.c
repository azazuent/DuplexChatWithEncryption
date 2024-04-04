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
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s hostname port", basename(argv[0]));
    }
    
    int fd, rv;

    char* hostname = argv[1];
    char* port = argv[2];

    struct addrinfo hints, *srvinfo, *p;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (rv = getaddrinfo(hostname, port, &hints, &srvinfo) != 0)
    {
        fprintf(stderr, "Selectserver: %s", gai_strerror(rv));
        exit(-1);
    }

    for(p = srvinfo; p != NULL; p = p->ai_next)
    {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(srvinfo);

    int pid = fork();

    if (pid < 0)
    {
        perror("Creating child process failed");
        exit(-1);
    }

    if (pid == 0)
        wait_send(&fd);
    else
        wait_recv(&fd);

    return 0;
}
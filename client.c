#include "utils.c"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s hostname port\n", basename(argv[0]));
        exit(-1);
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

        printf("Connection established\n");
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
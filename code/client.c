#include "utils.c"

int main(int argc, char *argv[])
{   
    int fd, rv;

    char *hostname = argv[1];
    char *port = argv[2];
    char mode = argv[3][0];

    DES_cblock *key = malloc(8);

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

    if (mode == '0') key = NULL;
    else if (mode == '1')
    {
        FILE *key_file = fopen(KEY_FILE_NAME, "rb");
        if (key_file == NULL)
        {
            printf("Couldn't read key from file\n");
            exit(-1);
        }
        if (fread(key, sizeof(DES_cblock), 1, key_file) != 1)
        {
            fclose(key_file);
            printf("Couldn't read key from file\n");
            exit(-1);
        }
        fclose(key_file);
    }
    else if (mode == '2')
    {
        
    }

    int pid = fork();

    if (pid < 0)
    {
        perror("Creating child process failed");
        exit(-1);
    }

    if (pid == 0)
        wait_send(&fd, key);
    else
        wait_recv(&fd, key);

    return 0;
}
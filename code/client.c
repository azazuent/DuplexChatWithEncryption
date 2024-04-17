#include "utils.c"

int main(int argc, char *argv[])
{   
    int fd, rv;

    char *hostname = argv[1];
    char *port = argv[2];
    char mode = argv[3][0];

    DES_cblock *key = malloc(sizeof(DES_cblock));

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

    if (mode == '0')
    {
        key = NULL;
        printf("Chatting with no encryption, be careful!\n");
    }
    else if (mode == '1')
    {
        if (read_key_from_file(key, KEY_FILE_NAME) != 0)
        {
            printf("Could not read key from file, sorry\n");
            exit(-1);
        }
        
        printf("Key read from %s, begin chatting!\n", KEY_FILE_NAME);
    }
    else if (mode == '2')
    {
        printf("Performing Diffie-Hellmann key exchange...\n");

        if (perform_dh_exchange(&fd, key) != 0)
        {
            printf("Could not perform key exchange\n");
            exit(-1);
        }
        BIO_dump_fp(stdout, key, sizeof(key));
        printf("Exchange successful, begin chatting!\n");
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
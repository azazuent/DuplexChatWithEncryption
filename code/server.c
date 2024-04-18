#include "utils.c"

int main(int argc, char *argv[])
{
    char *port = argv[1];
    char mode = argv[2][0];

    DES_cblock *key = malloc(sizeof(DES_cblock));

    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (rv = getaddrinfo(NULL, port, &hints, &ai) != 0)
    {
        fprintf(stderr, "Selectserver: %s", gai_strerror(rv));
        exit(-1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Selectserver: failed to bind\n");
        exit(-1);
    }

    freeaddrinfo(ai);

    if(listen(listener, 1) == -1)
    {
        perror("listen");
        exit(-1);
    }

    printf("Waiting for incoming connection\n");
    int fd;

    while(1)
    {
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen = sizeof(remoteaddr);

        fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
        if (fd == -1) 
        {
            perror("accept");
            continue;
        }

        char remoteIP[INET6_ADDRSTRLEN];
        inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, sizeof(remoteIP));
        printf("Connection established with %s\n", remoteIP);
        close(listener);
        break;
    }

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

        printf("Exchange successful, begin chatting!\n");
    }

    int result;
    pid_t pid = fork();
        
    if (pid < 0)
    {
        perror("Creating child process failed");
        exit(-1);
    }

    if (pid == 0)
        wait_send(&fd, key);
    else
    {
        result = wait_recv(&fd, key);
        kill(pid, SIGTERM);
    }

    return 0;
}
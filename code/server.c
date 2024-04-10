#include "utils.c"

int main(int argc, char *argv[])
{
    char *port = argv[1];
    char mode = argv[2][0];

    DES_cblock *key = malloc(8);

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
        printf("Key read from %s, begin chatting!\n", KEY_FILE_NAME);
    }
    else if (mode == '2')
    {
        printf("Performing Diffie-Hellmann key exchange...\n");

        char pub_key[256];

        DH* dh = DH_new();

        DH_generate_parameters_ex(dh, 1024, DH_GENERATOR_2, 0);
        DH_generate_key(dh);

        char* hex_key = BN_bn2hex(DH_get0_pub_key(dh));

        send(fd, hex_key, sizeof(hex_key), 0);
        recv(fd, hex_key, sizeof(hex_key), 0);

        BIGNUM *bn_key = BN_new();
        BN_hex2bn(&bn_key, hex_key);

        unsigned char* key = malloc(sizeof(dh));
        DH_compute_key((unsigned char*)key, bn_key, dh);

        printf("Exchange successful, begin chatting!\n");
    }

    pid_t pid = fork();
        
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
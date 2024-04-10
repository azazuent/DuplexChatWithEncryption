#include "utils.c"

int main(int argc, char *argv[])
{   
    int fd, rv;

    char *hostname = argv[1];
    char *port = argv[2];
    char mode = argv[3][0];

    DES_cblock *key = malloc(256);

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
        char pub_key[256];
        
        printf("Ok\n");

        DH* dh = DH_new();

        printf("Ok\n");

        DH_generate_parameters_ex(dh, 1024, DH_GENERATOR_2, 0);
        DH_generate_key(dh);
        
        printf("Ok\n");

        char* hex_key = BN_bn2hex(DH_get0_pub_key(dh));
        send(fd, hex_key, sizeof(hex_key), 0);
        recv(fd, hex_key, sizeof(hex_key), 0);
        BIGNUM *bn_key = BN_new();
        BN_hex2bn(&bn_key, hex_key);

        unsigned char* key = malloc(sizeof(dh));
        
        printf("Ok\n");

        DH_compute_key((unsigned char*)key, bn_key, dh);
        
        printf("Ok\n");
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
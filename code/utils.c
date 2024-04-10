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

#include <openssl/des.h>
#include <openssl/err.h>

#include <openssl/dh.h>
#include <openssl/engine.h>
#include <openssl/bn.h>

#define MAX_DATA_SIZE 256
#define KEY_FILE_NAME "des_key.bin"

const BIGNUM *DH_get0_pub_key(const DH *dh);

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int DES_crypto(const char* str, char* enc_str, DES_cblock* key, int encrypt)
{
    if (key == NULL)
    {
        strcpy(enc_str, str);
        return 0;
    }

    DES_key_schedule schedule;

    DES_set_key_unchecked(key, &schedule);

    for (int i = 0; i < MAX_DATA_SIZE / 8; i++)
    {
        const char *current_block = str + i * 8;
        char *dest_block = enc_str + i * 8;
        DES_ecb_encrypt((const_DES_cblock *)current_block, (DES_cblock *)dest_block, &schedule, encrypt);

    }
    return 0;
}

void wait_send(int *fd, DES_cblock* key)
{
    char sendbuf[MAX_DATA_SIZE];
    char sendbuf_enc[MAX_DATA_SIZE];
    while(1)
    {
        memset(sendbuf, 0, MAX_DATA_SIZE);
        memset(sendbuf_enc, 0, MAX_DATA_SIZE);
        fgets(sendbuf, MAX_DATA_SIZE, stdin);

        if (strcmp(sendbuf, "\n") == 0)
            continue;

        if (DES_crypto(sendbuf, sendbuf_enc, key, DES_ENCRYPT) != 0)
        {
            printf("Failed to encrypt message:\n");
            continue;
        }

        if (send(*fd, sendbuf_enc, MAX_DATA_SIZE, 0) < 0)
        {
            printf("Send failed\n");
            exit(1);
        }
        printf("Sent: %s", sendbuf);
    }
}

void wait_recv(int *fd, DES_cblock* key)
{
    char recvbuf[MAX_DATA_SIZE];
    char recvbuf_dec[MAX_DATA_SIZE];
    ssize_t msg_length;
    while(1)
    {
        memset(recvbuf, 0, MAX_DATA_SIZE);
        memset(recvbuf_dec, 0, MAX_DATA_SIZE);
        if (msg_length = recv(*fd, recvbuf, MAX_DATA_SIZE, 0) == -1)
        {
            perror("recv");
            exit(1);
        }

        if (recvbuf[0] == '\0')
        {
            printf("Client disconnected, exiting...\n");
            close(*fd);
            exit(0);
        }

        if (DES_crypto(recvbuf, recvbuf_dec, key, DES_DECRYPT) != 0)
        {
            printf("Failed to decrypt message\n");
            continue;
        }

        printf("Received: %s", recvbuf_dec);
    }
}

void prepare_DH_key(char *key)
{
    DH *dh = DH_new();
    int res = DH_generate_parameters_ex(dh, 1024, DH_GENERATOR_2, 0);
    DH_generate_key(dh);

    char *prepared_key = BN_bn2hex(DH_get0_pub_key(dh));
    strcpy(key, prepared_key);
}

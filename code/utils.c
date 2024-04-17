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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int read_key_from_file(DES_cblock* key, const char* file_name)
{
    FILE *key_file = fopen(file_name, "rb");
    if (key_file == NULL)
    {
        printf("Couldn't read key from file\n");
        return -1;
    }
    if (fread(key, sizeof(DES_cblock), 1, key_file) != 1)
    {
        fclose(key_file);
        printf("Couldn't read key from file\n");
        return -1;
    }
    fclose(key_file);

    return 0;
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

int perform_dh_exchange(int* fd, DES_cblock* key)
{
    DH* dh = DH_new();

    if (DH_generate_parameters_ex(dh, 1024, DH_GENERATOR_2, 0) != 1) return -1;
    if (DH_generate_key(dh) != 1) return -1;

    char* hex_key = BN_bn2hex(DH_get0_pub_key(dh));

    send(*fd, hex_key, sizeof(hex_key), 0);
    recv(*fd, hex_key, sizeof(hex_key), 0);

    BIGNUM *bn_key = BN_new();
    if (BN_hex2bn(&bn_key, hex_key) == 0) return -1;

    unsigned char* dh_key = malloc(sizeof(dh));

    if (DH_compute_key(dh_key, bn_key, dh) == -1) return -1;

    printf("Okay\n");

    strncpy((char *)key, (char *)dh_key, sizeof(DES_cblock));

    printf("Okay\n");

    DH_free(dh);
    BN_free(bn_key);

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

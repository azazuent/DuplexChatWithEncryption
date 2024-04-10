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

#define MAX_DATA_SIZE 128
#define KEY_FILE_NAME "des_key.bin"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int encryptDES(char* buf, char* enc_buf)
{
    DES_cblock key;
    DES_key_schedule schedule;

    FILE *key_file = fopen(KEY_FILE_NAME, "rb");
    if (key_file == NULL) return -1;

    if (fread(key, sizeof(DES_cblock), 1, key_file) != 1)
    {
        fclose(key_file);
        return -1;
    }
    fclose(key_file);
    //printf("Key read successfully\n");

    DES_set_key_unchecked(&key, &schedule);
    //printf("Key set successfully\n");

    DES_ecb_encrypt((const_DES_cblock *)buf, (DES_cblock *)enc_buf, &schedule, DES_ENCRYPT);
    //printf("String encoded successfully\n");
    return 0;
}

int decryptDES(char* buf, char* dec_buf)
{
    DES_cblock key;
    DES_key_schedule schedule;

    FILE *key_file = fopen(KEY_FILE_NAME, "rb");
    if (key_file == NULL) return -1;

    if (fread(key, sizeof(DES_cblock), 1, key_file) != 1)
    {
        fclose(key_file);
        return -1;
    }
    fclose(key_file);
    //printf("Key read successfully\n");

    DES_set_key_unchecked(&key, &schedule);
    //printf("Key set successfully\n");

    DES_ecb_encrypt((const_DES_cblock *)buf, (DES_cblock *)dec_buf, &schedule, DES_DECRYPT);
    //printf("String decoded successfully\n");
    return 0;
}

void wait_send(int *fd)
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

        if (encryptDES(sendbuf, sendbuf_enc) != 0)
        {
            printf("Failed to encrypt message:\n");
            continue;
        }
        printf("Original strlen: %lu\n", strlen(sendbuf));
        printf("New strlen: %lu\n", strlen(sendbuf_enc));
        printf("Sizeof: %lu\n", sizeof(sendbuf_enc));

        if (send(*fd, sendbuf_enc, sizeof(sendbuf_enc), 0) < 0)
        {
            printf("Send failed\n");
            exit(1);
        }
        printf("Sent: %s", sendbuf);
    }
}

void wait_recv(int *fd)
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

        if (decryptDES(recvbuf, recvbuf_dec) != 0)
        {
            printf("Failed to decrypt message\n");
            continue;
        }

        printf("Size from recv: %ld from strlen: %lu\n", msg_length, strlen(recvbuf));
        printf("Received: %s", recvbuf_dec);
    }
}
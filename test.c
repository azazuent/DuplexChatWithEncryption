#include "code/utils.c"

int main(int argc, char *argv[])
{
    DH* dh1 = DH_new();
    DH* dh2 = DH_new();

    printf("Ok\n");

    if (DH_generate_parameters_ex(dh1, 1024, DH_GENERATOR_2, 0) != 1) printf("FUUUUCK");
    //if (DH_check_ex(dh1) != 1) printf("FUUUUCK");
    DH_generate_key(dh1);

    printf("Ok\n");

    if (DH_generate_parameters_ex(dh2, 1024, DH_GENERATOR_2, 0) != 1) printf("FUUUUCK");
    //if (DH_check_ex(dh2) != 1) printf("FUUUUCK");
    DH_generate_key(dh2);

    printf("Ok\n");

    unsigned char* key1 = malloc(sizeof(dh1));
    unsigned char* key2 = malloc(sizeof(dh2));

    const BIGNUM *bn1 = DH_get0_pub_key(dh1);
    const BIGNUM *bn2 = DH_get0_pub_key(dh2);
    
    printf("Ok\n");

    if (DH_compute_key(key1, bn2, dh1) == -1) printf("FUUUUCK");
    if (DH_compute_key(key2, bn1, dh2) == -1) printf("FUUUUCK2");
    
    printf("Ok\n");
    
    BIO_dump_fp(stdout, key1, sizeof(key1));
    BIO_dump_fp(stdout, key2, sizeof(key2));

    //DH_free(dh1);
    //DH_free(dh2);

    return 0;
}

#include "utils.c"

int main(int argc, char *argv[])
{
    // Создание контекста клиента, инициализирующего подключение 
    DH* dh1 = DH_new();

    // Генерация параметров ДХ
    if (DH_generate_parameters_ex(dh1, 1024, DH_GENERATOR_2, 0) != 1);
    // Проверка корректности параметров ДХ (p должно быть простым)
    if (DH_check_ex(dh1) != 1);

    // Достаем параметры ДХ из контекста первого клиента
    const BIGNUM *p1, *q1, *g1;
    DH_get0_pqg(dh1, &p1, &q1, &g1);

    // Создаем копии параметров для использования вторым клиентом
    BIGNUM *p2 = BN_dup(p1);
    BIGNUM *q2 = BN_dup(q1);
    BIGNUM *g2 = BN_dup(g1);

    // Где-то здесь передаем условные p2, q2, g2 второму клиенту

    // Создание контекста второго клиента
    DH* dh2 = DH_new();
    // Выставляем параметры, идентичные первому клиенту
    DH_set0_pqg(dh2, p2, q2, g2);
    if (DH_check_ex(dh2) != 1) printf("FUUUUCK");

    // Генерируем открытые ключи
    DH_generate_key(dh1);
    DH_generate_key(dh2);

    printf("Ok\n");

    // Достаем открытые ключи из контекстов
    const BIGNUM *bn1 = DH_get0_pub_key(dh1);
    const BIGNUM *bn2 = DH_get0_pub_key(dh2);
    
    printf("Ok\n");

    // Где-то здесь клиенты обмениваются сгенерированными ключами

    // Рассчитываем общие секреты по протоколу ДХ
    unsigned char* key1 = malloc(DH_size(dh1));
    unsigned char* key2 = malloc(DH_size(dh2));

    if (DH_compute_key(key1, bn2, dh1) == -1) printf("FUUUUCK\n");
    if (DH_compute_key(key2, bn1, dh2) == -1) printf("FUUUUCK2\n");
    printf("Ok\n");

    // Визуальное сравнение ключей, чтобы убедиться в успехе    
    BIO_dump_fp(stdout, key1, sizeof(key1));
    BIO_dump_fp(stdout, key2, sizeof(key2));

    DH_free(dh1);
    DH_free(dh2);

    free(key1);
    free(key2);

    return 0;
}

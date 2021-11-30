#include "client.h"


// client et server font sur a.param
int client_init(client * client) {
    // inspiré de pbc_demo_pairing_init( de pbc_test.h
    pairing_t pairing;
    char s[16384];
    FILE *fp = fopen("a.param", "r");
    if (!fp) pbc_die("error opening %s", "a.param");

    size_t count = fread(s, 1, 16384, fp);
    if (!count) pbc_die("input error");
    fclose(fp);
    if (pairing_init_set_buf(client->pairing, s, count)) pbc_die("pairing init failed");
    
    // choix d'un générateur commun :

    // initialisation de data :
    profil_random(&client->data, 10);

    // initialisation de hashed :

    // initialisation de signatures :


}

int main(int argc, char* argv[]) {
    test_pbc();
    return 0;
}
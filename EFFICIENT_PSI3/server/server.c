#include "server.h"

// client et server font sur a.param
int server_init(server * server) {
    // inspiré de pbc_demo_pairing_init de pbc_test.h
    char s[16384];
    FILE *fp = fopen("a.param", "r");
    if (!fp) pbc_die("error opening %s", "a.param");

    size_t count = fread(s, 1, 16384, fp);
    if (!count) pbc_die("input error");
    fclose(fp);
    if (pairing_init_set_buf(server->pairing, s, count)) pbc_die("pairing init failed");
    
    // choix d'un générateur commun :

    element_init_G1(server->g, server->pairing);
    element_from_hash(server->g, &generator_hash, 10);

    // initialisation de data :
    profil_random(&server->data, RANDOM_DATA);

    hash_pair test = {4, 3};
    profil_append(&server->data, &test);
    hash_pair test2 = {6, 0};
    profil_append(&server->data, &test2);

    // initialisation de hashed :
    // profil_hashed(&server->data, &server->hashed);
    profil_hashed(&server->data);

    return  0;
}

int server_clear(server * server) {
    profil_clear(&server->data);
    element_clear(server->g);
    pairing_clear(server->pairing);
    return 0;
}

element_t * server_receive_round2(int fd, int * v, pairing_t * pairing) {  // pas oublier de faire le malloc pour y
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE); // on clean le malloc dans le main
    // on reçoit d'abord la taille (remplit v)
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    sscanf((char*) buffer, "%d", v);
    // *v = atoi((char*) buffer);

    // printf("v is : %d\n", *v);

    // on reçoit les y en faisant un malloc
    element_t * y = (element_t *) malloc(sizeof(element_t) * (*v));
    for (int i = 0; i< *v; i++) {
        element_init_G1(y[i], *pairing);
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        fd_read(fd, buffer, ELEMENT_BUF_SIZE);
        // printf("buffer : %s\n", buffer);
        element_from_bytes(y[i], buffer);
        // element_printf("yi  : %B\n", y[i]);
    }
    return y;
}

int server_send_round4(int fd, int v, int w, unsigned char * nonce, element_t* y, unsigned long * t) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    
    // on envoie w
    sprintf((char*) buffer, "%d", w);
    fd_write(buffer, fd, ELEMENT_BUF_SIZE);

    // on envoie nonce
    fd_write(nonce, fd, NONCE_SIZE);

    // on envoie les y
    for (int i = 0; i<v; i++) {
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        int gen_size = element_to_bytes(buffer, y[i]);
        if (gen_size < ELEMENT_BUF_SIZE) {
            return -1;
        }
        // printf("buffer : %s\n", buffer);
        fd_write(buffer, fd, gen_size); 
    }

    // on envoie les t 
    for (int i = 0; i<w; i++) {
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        sprintf((char*) buffer, "%lu", t[i]);
        fd_write(buffer, fd, ELEMENT_BUF_SIZE);
    }


    return 0;
}

int main(int argc, char* argv[]) {
    // initialisation des données du server
    server server_test;
    server_init(&server_test);
    // element_printf("generator : %B\n", server_test.g);
    printf("Data :\n");
    profil_print(stdout, &server_test.data);

    // génération de clef privée et clef publique. 
    // element_t priv_k, inv, pub_k;
    // element_init_Zr(priv_k, server_test.pairing);
    // element_init_Zr(inv, server_test.pairing);
    // element_init_G1(pub_k, server_test.pairing);
    // element_random(priv_k);
    // element_invert(inv, priv_k);
    // element_pow_zn(pub_k, server_test.g, inv);
    // unsigned char pub_k_bytes[128];
    // memset(pub_k_bytes, 0, 128);
    // unsigned char priv_k_bytes[20];
    // memset(priv_k_bytes, 0, 20);
    // element_to_bytes(pub_k_bytes, pub_k);
    // printf("test : %s\n", pub_k_bytes); 
    // printf("test : %s\n", priv_k_bytes); 
    // element_to_bytes(priv_k_bytes, priv_k);
    // FILE *fp1 = fopen("pub_k.txt", "w+");
    // if (!fp1) pbc_die("error opening %s", "pub_k.txt");
    // fwrite(pub_k_bytes, 1, 128, fp1);
    // fclose(fp1);
    // FILE *fp2 = fopen("priv_k.txt", "w+");
    // if (!fp2) pbc_die("error opening %s", "priv_k.txt");
    // fwrite(priv_k_bytes, 1, 20, fp2);
    // fclose(fp2);
    // element_printf("priv_k : %B \ninv : %B \npub_k : %B\n", priv_k, inv, pub_k);  
    
    element_t priv_k, pub_k;
    element_init_Zr(priv_k, server_test.pairing);
    element_init_G1(pub_k, server_test.pairing);
    unsigned char pub_k_bytes[128];
    unsigned char priv_k_bytes[20];

    // On a généré la clef publique et privée au préalable grâce au code ci-dessus. On récupère ces clefs pour l'execution. 
    // private key = x, public key = g^(1/x)
    FILE *fp1 = fopen("pub_k.txt", "r");
    if (!fp1) pbc_die("error opening %s", "pub_k.txt");
    fread(pub_k_bytes, 128, 1, fp1);
    fclose(fp1);
    FILE *fp2 = fopen("priv_k.txt", "r");
    if (!fp2) pbc_die("error opening %s", "priv_k.txt");
    fread(priv_k_bytes, 20, 1, fp2);
    fclose(fp2);

    element_from_bytes(pub_k, pub_k_bytes);
    element_from_bytes(priv_k, priv_k_bytes);

    
    // element_printf("\npriv_k : %B \npub_k : %B\ntest : %B\n", priv_k, pub_k, test); 


    // mise en place connection tcp
    printf("\n");
    int fd = socket_connect_server(PORT_SERVER);

    if (fd < 0) {
        return -1;
    }

    clock_t begin = clock();

    // Round 1 (client)
    printf("\n\nRound 1 (client) :\n");

    clock_t end1 = clock();
    printf("temps exec round 1 (ms) = %f\n", ((double)(end1 - begin) / CLOCKS_PER_SEC) * 1000);
    // Round 2

    printf("\n\nRound 2 :\n");

    int v;
    // on reçoit la taille et les y
    element_t * y = server_receive_round2(fd, &v, &server_test.pairing); // tableau de taille v
    clock_t end2 = clock();
    printf("temps exec round 2 (ms) = %f\n", ((double)(end2 - end1) / CLOCKS_PER_SEC) * 1000);


    // Round 3

    printf("\n\nRound 3 :\n");

    element_t hsj;
    element_init_G1(hsj, server_test.pairing);

    unsigned char data[ELEMENT_BUF_SIZE + NONCE_SIZE];
    unsigned char data_hashed[8];
    unsigned char nonce[NONCE_SIZE];

    for (int i = 0; i<NONCE_SIZE; i++)  {
        nonce[i] = (unsigned char) rand();
    }

    unsigned long * t = (unsigned long *) malloc(sizeof(unsigned long) * server_test.data.tab->nb_elts);

    int index = 0;
    for (int j = 0; j<server_test.data.tab->size; j++) {
        if (hash_is_defined(server_test.data.tab, j)) {
            element_from_hash(hsj , &server_test.data.tab->table[j].v, ELEMENT_BUF_SIZE); //
            element_pow_zn(hsj, hsj, priv_k);
            element_to_bytes(data, hsj);
            for (int i = 0 ; i<NONCE_SIZE; i++) {
                data[ELEMENT_BUF_SIZE + i] = nonce[i];
            }
            // on hash hs_j^x || nonce
            sha3(data, ELEMENT_BUF_SIZE + NONCE_SIZE, data_hashed, 8);
            memcpy(&t[index], data_hashed, sizeof(unsigned long));
            index += 1;
        }
    }

    for (int i = 0; i<v; i++) {
        element_pow_zn(y[i], y[i], priv_k); // y[i] = \tilde{y_i}
    }

    clock_t end3 = clock();
    printf("temps exec round 3 (ms) = %f\n", ((double)(end3 - end2) / CLOCKS_PER_SEC) * 1000);

    // Round 4

    printf("\n\nRound 4 :\n");


    server_send_round4(fd, v, server_test.data.tab->nb_elts, nonce, y, t);

    clock_t end4 = clock();
    printf("temps exec round 4 (ms) = %f\n", ((double)(end4 - end3) / CLOCKS_PER_SEC) * 1000);

    // Round 5 (client)

    printf("\n\nRound 5 (client) :\n");

    close(fd);

    // clear des éléments
    element_clear(hsj);

    // clear des tableaux
    for (int i = 0; i<v; i++) {
        element_clear(y[i]);
    }
    free(y);
    free(t);
    server_clear(&server_test);
    return 0;
}
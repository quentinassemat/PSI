#include "client.h"


// client et server font sur a.param
int client_init(client * client) {
    // inspiré de pbc_demo_pairing_init de pbc_test.h
    char s[16384];
    FILE *fp = fopen("a.param", "r");
    if (!fp) pbc_die("error opening %s", "a.param");

    size_t count = fread(s, 1, 16384, fp);
    if (!count) pbc_die("input error");
    fclose(fp);
    if (pairing_init_set_buf(client->pairing, s, count)) pbc_die("pairing init failed");
    
    // choix d'un générateur commun :

    element_init_G1(client->g, client->pairing);
    element_from_hash(client->g, &generator_hash, 10);

    // initialisation de data :
    profil_random(&client->data, RANDOM_DATA);

    hash_pair test = {4, 3};
    profil_append(&client->data, &test);
    hash_pair test2 = {6, 0};
    profil_append(&client->data, &test2);

    // initialisation de hashed :
    profil_hashed(&client->data);

    return  0;
}


int client_clear(client * client) {
    profil_clear(&client->data);
    element_clear(client->g);
    pairing_clear(client->pairing);
    return 0;
}

// on envoie le X et les y_i par tcp
int client_send_round2(int fd, int v, element_t* y) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    
    // on envoie v
    sprintf((char*) buffer, "%d", v);
    fd_write(buffer, fd, ELEMENT_BUF_SIZE);

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
    return 0;
}

unsigned long * client_receive_round4(int fd, int v, int * w, unsigned char * nonce, element_t * y2, pairing_t * pairing) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);

    // on reçoit d'abord la taille (remplit w)
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    // *w = atoi((char*) buffer);
    sscanf((char*) buffer, "%d", w);

    // printf("w is : %d\n", *w);

    // on reçoit nonce 
    fd_read(fd, nonce, NONCE_SIZE);

    // on reçoit les y2
    for (int i = 0; i< v; i++) {
        element_init_G1(y2[i], *pairing);
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        fd_read(fd, buffer, ELEMENT_BUF_SIZE);
        // printf("buffer : %s\n", buffer);
        element_from_bytes(y2[i], buffer);
        // element_printf("yi  : %B\n", y[i]);
    }

    // on reçoit les t en faisant un malloc
    unsigned long * t = (unsigned long *) malloc (sizeof(unsigned long) * (*w));
    for (int j = 0; j< *w; j++) {
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        fd_read(fd, buffer, ELEMENT_BUF_SIZE);
        sscanf( (char*) buffer, "%lu", &t[j]);
    }
    return t;
}

int main(int argc, char* argv[]) {

    // initialisation des données du client
    client client_test;
    client_init(&client_test);
    element_printf("generator : %B\n", client_test.g);
    printf("\nData :\n");
    profil_print(stdout, &client_test.data);

    element_t pub_k;
    element_init_G1(pub_k, client_test.pairing);
    unsigned char pub_k_bytes[128];

    // On a généré la clef publique et privée au préalable grâce au code ci-dessus. On récupère ces clefs pour l'execution. 
    // private key = x, public key = g^(1/x)
    FILE *fp = fopen("pub_k.txt", "r");
    if (!fp) pbc_die("error opening %s", "pub_k.txt");
    fread(pub_k_bytes, 128, 1, fp);
    fclose(fp);

    element_from_bytes(pub_k, pub_k_bytes);

    // mise en place connection tcp :
    int fd = socket_connect(PORT_SERVER);
    if (fd < 0) {
        return fd;
    }

    // Round 1

    clock_t begin = clock();

    printf("\n\nRound 1 :\n");

    element_t hci; 
    element_t * y = (element_t *)  malloc(sizeof(element_t) * client_test.data.tab->nb_elts);
    element_t * alpha = (element_t *) malloc(sizeof(element_t) * client_test.data.tab->nb_elts);

    element_init_G1(hci, client_test.pairing);

    int index = 0;

    // on calcule les y_i
    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            element_init_Zr(alpha[index], client_test.pairing);
            element_random(alpha[index]);
            element_init_G1(y[index], client_test.pairing);
            element_from_hash(hci , &client_test.data.tab->table[i].v, ELEMENT_BUF_SIZE);
            element_pow_zn(y[index], pub_k, alpha[index]);
            element_mul(y[index], y[index], hci);
            index += 1;
        }
    }


    clock_t end1 = clock();
    printf("temps exec round 1 (ms) = %f\n", ((double)(end1 - begin) / CLOCKS_PER_SEC) * 1000);

    // Round 2

    printf("\n\nRound 2 :\n");

    client_send_round2(fd, client_test.data.tab->nb_elts, y);


    clock_t end2 = clock();
    printf("temps exec round 2 (ms) = %f\n", ((double)(end2 - end1) / CLOCKS_PER_SEC) * 1000);

    // Round 3 (server)

    printf("\n\nRound 3 (server) :\n");


    clock_t end3 = clock();
    printf("temps exec round 3 (ms) = %f\n", ((double)(end3 - end2) / CLOCKS_PER_SEC) * 1000);

    // Round 4

    printf("\n\nRound 4 :\n");

    int w;

    element_t * y2 = (element_t *)  malloc(sizeof(element_t) * client_test.data.tab->nb_elts);
    unsigned char nonce[NONCE_SIZE];

    unsigned long * t = client_receive_round4(fd, client_test.data.tab->nb_elts, &w, nonce, y2, &client_test.pairing);

    // for (int i = 0; i<w; i++) {
    //     printf("t[%d] : %lu\n", i, t[i]);
    // }


    clock_t end4 = clock();
    printf("temps exec round 4 (ms) = %f\n", ((double)(end4 - end3) / CLOCKS_PER_SEC) * 1000);

    // Round 5

    printf("\n\nRound 5 :\n");

    unsigned long * t2 = (unsigned long *) malloc(sizeof(unsigned long) * client_test.data.tab->nb_elts);

    index = 0;
    element_t g_alpha_i;
    element_init_G1(g_alpha_i, client_test.pairing);
    unsigned char data[ELEMENT_BUF_SIZE];
    unsigned char data_hashed[8];

    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        element_pow_zn(g_alpha_i, client_test.g, alpha[i]);
        element_div(y2[i], y2[i], g_alpha_i);
        element_to_bytes(data, y2[i]);
        for (int i = 0 ; i<NONCE_SIZE; i++) {
            data[ELEMENT_BUF_SIZE + i] = nonce[i];
        }
        sha3(data, ELEMENT_BUF_SIZE + NONCE_SIZE, data_hashed, 8);
        memcpy(&t2[i], data_hashed, sizeof(unsigned long));
        // printf("t2[%d] : %lu\n", index, t2[index]);
    }


    int* index_intersection = (int *) malloc(sizeof(int) * client_test.data.tab->nb_elts);
    index = 0;
    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        for (int j = 0; j < w; j++) {
            if (t[j] == t2[i]) {
                index_intersection[index ++] = i;
            }
        }
    }

    printf("\nOUTPUT : \n");

    int index2 = 0;
    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            for (int j = 0; j<index; j++) {
                if (index2 == index_intersection[j]) {
                    printf("%lu %lu\n", client_test.data.tab->table[i].k, client_test.data.tab->table[i].v);
                }
            }
            index2 += 1;
        }
    }


    clock_t end5 = clock();
    printf("temps exec round 5 (ms) = %f\n", ((double)(end5 - end4) / CLOCKS_PER_SEC) * 1000);

    close(fd);

    // clear des éléments 
    element_clear(hci);


    // clear des tableaux
    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        element_clear(y[i]);
        element_clear(y2[i]);
    }
    free(y);
    free(y2);
    free(t);

    client_clear(&client_test);

    return 0;
}
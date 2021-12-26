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
int client_send_round2(int fd, int v, element_t* X, element_t* y) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    
    // on envoie v
    sprintf((char*) buffer, "%d", v);
    fd_write(buffer, fd, ELEMENT_BUF_SIZE);

    // on envoie X
    memset(buffer, 0, ELEMENT_BUF_SIZE);
    int gen_size = element_to_bytes(buffer, *X);
    if (gen_size < ELEMENT_BUF_SIZE) {
        return -1;
    }
    fd_write(buffer, fd, gen_size);

    // on envoie les y 
    for (int i = 0; i<v; i++) {
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        gen_size = element_to_bytes(buffer, y[i]);
        if (gen_size < ELEMENT_BUF_SIZE) {
            return -1;
        }
        // printf("buffer : %s\n", buffer);
        fd_write(buffer, fd, gen_size); 
    }
    return 0;
}

unsigned long * client_receive_round4(int fd, int v, int * w, element_t* Z, element_t * y2, pairing_t * pairing) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    // on reçoit d'abord la taille (remplit w)
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    // *w = atoi((char*) buffer);
    sscanf((char*) buffer, "%d", w);

    // printf("w is : %d\n", *w);

    // on reçoit Z
    memset(buffer, 0, ELEMENT_BUF_SIZE);
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    element_from_bytes(*Z, buffer);

    // element_printf("Z is : %B\n", *Z);

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
    // mise en place connection tcp :
    int fd = socket_connect(PORT_SERVER);
    if (fd < 0) {
        return fd;
    }

    // initialisation des données du client
    client client_test;
    client_init(&client_test);
    element_printf("generator : %B\n", client_test.g);
    printf("\nData :\n");
    profil_print(stdout, &client_test.data);

    // Round 1

    clock_t begin = clock();

    printf("\n\nRound 1 :\n");

    element_t PCH, X, Rc, hci; 
    element_t * y = (element_t *)  malloc(sizeof(element_t) * client_test.data.tab->nb_elts);
    element_t * Rci = (element_t *) malloc(sizeof(element_t) * client_test.data.tab->nb_elts);

    element_init_G1(PCH, client_test.pairing);
    element_init_G1(X, client_test.pairing);
    element_init_Zr(Rc, client_test.pairing);
    element_init_G1(hci, client_test.pairing);

    int index = 0;

    // on calcule PCH
    element_set1(PCH);
    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            element_init_G1(Rci[index], client_test.pairing);
            element_init_G1(y[index++], client_test.pairing);
            element_from_hash(hci , &client_test.data.tab->table[i].v, ELEMENT_BUF_SIZE);
            element_mul(PCH, PCH, hci);
        }
    }

    // on choisi random Rc
    element_random(Rc);

    // on calcule X
    element_pow_zn(X, client_test.g, Rc);
    element_mul(X, X, PCH);

    // element_printf("X is : %B", X);

    index = 0;
    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            element_from_hash(hci , &client_test.data.tab->table[i].v, ELEMENT_BUF_SIZE);
            element_random(Rci[index]); // Rci
            element_pow_zn(y[index], client_test.g, Rci[index]);
            element_mul(y[index], y[index], PCH);
            element_div(y[index], y[index], hci);
            index += 1;
        }
    }

    clock_t end1 = clock();
    printf("temps exec round 1 (ms) = %f\n", ((double)(end1 - begin) / CLOCKS_PER_SEC) * 1000);

    // Round 2

    printf("\n\nRound 2 :\n");

    client_send_round2(fd, client_test.data.tab->nb_elts, &X, y);


    clock_t end2 = clock();
    printf("temps exec round 2 (ms) = %f\n", ((double)(end2 - end1) / CLOCKS_PER_SEC) * 1000);

    // Round 3 (server)

    printf("\n\nRound 3 (server) :\n");


    clock_t end3 = clock();
    printf("temps exec round 3 (ms) = %f\n", ((double)(end3 - end2) / CLOCKS_PER_SEC) * 1000);

    // Round 4

    printf("\n\nRound 4 :\n");

    int w;
    element_t Z;
    element_init_G1(Z, client_test.pairing);

    element_t * y2 = (element_t *)  malloc(sizeof(element_t) * client_test.data.tab->nb_elts);

    unsigned long * t = client_receive_round4(fd, client_test.data.tab->nb_elts, &w, &Z, y2, &client_test.pairing);

    // for (int i = 0; i<w; i++) {
    //     printf("t[%d] : %lu\n", i, t[i]);
    // }


    clock_t end4 = clock();
    printf("temps exec round 4 (ms) = %f\n", ((double)(end4 - end3) / CLOCKS_PER_SEC) * 1000);
    // Round 5

    printf("\n\nRound 5 :\n");

    unsigned long * t2 = (unsigned long *) malloc(sizeof(unsigned long) * client_test.data.tab->nb_elts);


    index = 0;
    element_t Kci, exp;
    element_init_G1(Kci, client_test.pairing);
    element_init_Zr(exp, client_test.pairing);
    unsigned char data[ELEMENT_BUF_SIZE];
    unsigned char data_hashed[8];
    int len;

    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            element_sub(exp, Rc, Rci[index]);
            element_pow_zn(Kci, Z, exp);
            element_mul(Kci, Kci, y2[index]);
            len = element_to_bytes(data, Kci);
            sha3(data, len, data_hashed, 8);
            memcpy(&t2[index], data_hashed, sizeof(unsigned long));
            // printf("t2[%d] : %lu\n", index, t2[index]);
            index += 1;
        }
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
    element_clear(X);
    element_clear(Rc);
    element_clear(Z);
    element_clear(hci);
    element_clear(Kci);
    element_clear(exp);
    element_clear(PCH);

    // clear des tableaux
    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        element_clear(y[i]);
        element_clear(y2[i]);
        element_clear(Rci[i]);
    }
    free(y);
    free(y2);
    free(t);
    free(Rci);

    client_clear(&client_test);

    return 0;
}
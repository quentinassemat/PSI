#include "client.h"


// client et server font sur a.param
int client_init(client * client) {
    mpz_init_set_str(client->N, "179769313444375785804716951854212807170043487020564424359646658673359125268930341796385211192186165268958424981342683344803531250982610813881602937625776615479086270628175618202588941196085350840081586707369452911944254560327853117111162943558767673883041467956486674508460670039498430779083939927860702871553", 10);
    mpz_init_set_str(client->e, "59342290848241855381758259843496463780921957312662324296961624385704298102967", 10);
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
    return 0;
}

// on envoie le X et les y_i par tcp
int client_send_round3(int fd, int v, mpz_t* y) {
    uchar * buffer = malloc(sizeof(uchar) * MPZ_BUF_SIZE);
    
    // on envoie v
    sprintf((char*) buffer, "%d", v);
    fd_write(buffer, fd, MPZ_BUF_SIZE);

    // on envoie les y 
    for (int i = 0; i<v; i++) {
        memset(buffer, 0, MPZ_BUF_SIZE);
        // gmp_printf("y : %Zd\n", y[i]);
        mpz_export(buffer, NULL, -1, 1, 0, 0, y[i]);
        // printf("buffer : %s\n", buffer);
        fd_write(buffer, fd, MPZ_BUF_SIZE); 
    }

    free(buffer);
    return 0;
}

// unsigned long * t = (unsigned long *) malloc(sizeof(unsigned long) * server_test.data.tab->nb_elts);
unsigned long * client_receive_round5(int fd, int v, int * w, mpz_t * y2) {
    uchar * buffer = malloc(sizeof(uchar) * MPZ_BUF_SIZE);
    // on reçoit d'abord la taille (remplit w)
    fd_read(fd, buffer, MPZ_BUF_SIZE);
    // *w = atoi((char*) buffer);
    sscanf((char*) buffer, "%d", w);

    // printf("w is : %d\n", *w);

    // on reçoit les y2
    for (int i = 0; i< v; i++) {
        mpz_init(y2[i]);
        memset(buffer, 0, MPZ_BUF_SIZE);
        fd_read(fd, buffer, MPZ_BUF_SIZE);
        // printf("buffer : %s\n", buffer);
        mpz_import(y2[i], MPZ_BUF_SIZE, -1, 1, 0, 0, buffer);
    }

    // on reçoit les t en faisant un malloc
    unsigned long * t = (unsigned long *) malloc (sizeof(unsigned long) * (*w));
    for (int j = 0; j< *w; j++) {
        memset(buffer, 0, MPZ_BUF_SIZE);
        fd_read(fd, buffer, MPZ_BUF_SIZE);
        sscanf( (char*) buffer, "%lu", &t[j]);
        // printf("t_j : %lu\n", t[j]);
    }
    free(buffer);
    return t;
}

int main(int argc, char* argv[]) {
    // initialisation des données du client
    client client_test;
    client_init(&client_test);
    printf("\nData :\n");
    profil_print(stdout, &client_test.data);

    clock_t begin = clock();

    // Precomputation:
    printf("Precomputation :\n");
    mpz_t * y = (mpz_t *) malloc(sizeof(mpz_t) * client_test.data.tab->nb_elts);
    mpz_t * Rci = (mpz_t *) malloc(sizeof(mpz_t) * client_test.data.tab->nb_elts);
    mpz_t hci, bound;
    gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_inits(hci, bound, NULL);
    int index = 0;
    mpz_sub_ui(bound, client_test.N, 1);

    for (int i = 0; i<client_test.data.tab->size; i++) {
        if (hash_is_defined(client_test.data.tab, i)) {
            mpz_inits(y[index], Rci[index], NULL);
            mpz_urandomm(Rci[index], state, bound);
            mpz_add_ui(Rci[index], Rci[index], 1); // Rci random dans [1, N-1];
            mpz_set_ui(hci, (unsigned long int) client_test.data.tab->table[i].v);
            mpz_powm(y[index], Rci[index], client_test.e, client_test.N);
            mpz_mul(y[index], y[index], hci);
            mpz_mod(y[index], y[index], client_test.N);
            index += 1;
        }
    }
    mpz_clears(hci, bound, NULL);

    clock_t end0 = clock();
    printf("temps exec precomputation (ms) = %f\n", ((double)(end0 - begin) / CLOCKS_PER_SEC) * 100);


    // mise en place connection tcp :
    int fd = socket_connect(PORT_SERVER);
    if (fd < 0) {
        return fd;
    }

    // Round 3

    begin = clock();

    printf("\n\nRound 3 :\n");

    client_send_round3(fd, client_test.data.tab->nb_elts, y);

    clock_t end1 = clock();
    printf("temps exec round 3 (ms) = %f\n", ((double)(end1 - begin) / CLOCKS_PER_SEC) * 1000);

    // Round 4

    printf("\n\nRound 4 (Server) :\n");

    // Round 5 (server)

    clock_t end2 = clock();

    printf("\n\nRound 5 (server) :\n");

    int w;
    mpz_t * y2 = (mpz_t *)  malloc(sizeof(mpz_t) * client_test.data.tab->nb_elts);
    unsigned long * t = client_receive_round5(fd, client_test.data.tab->nb_elts, &w, y2);


    clock_t end3 = clock();
    printf("temps exec round 5 (ms) = %f\n", ((double)(end3 - end2) / CLOCKS_PER_SEC) * 1000);

    // Round 6

    printf("\n\nRound 6 :\n");
    unsigned long * t2 = (unsigned long *) malloc(sizeof(unsigned long) * client_test.data.tab->nb_elts);


    index = 0;
    mpz_t Kci, inv;
    mpz_inits(Kci, inv, NULL);
    unsigned char data[MPZ_BUF_SIZE];
    unsigned char data_hashed[8];

    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        mpz_invert(inv, Rci[i], client_test.N);
        mpz_mul(Kci, y2[i], inv);
        mpz_mod(Kci, Kci, client_test.N);
        memset(&data, 0, MPZ_BUF_SIZE);
        mpz_export(data, NULL, -1, 1, 0, 0, Kci);
        sha3(data, MPZ_BUF_SIZE, data_hashed, 8);
        memcpy(&t2[i], data_hashed, sizeof(unsigned long));
        // printf("ti : %lu\n", t2[i]);
    }

    clock_t end4 = clock();
    printf("temps exec round 6 (ms) = %f\n", ((double)(end4 - end3) / CLOCKS_PER_SEC) * 1000);

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
    printf("temps exec output (ms) = %f\n", ((double)(end5 - end4) / CLOCKS_PER_SEC) * 1000);

    close(fd);

    // clear des éléments 
    mpz_clears(Kci, inv, NULL);

    // clear des tableaux
    for (int i = 0; i<client_test.data.tab->nb_elts; i++) {
        mpz_clear(y[i]);
        mpz_clear(y2[i]);
        mpz_clear(Rci[i]);
    }
    free(y);
    free(y2);
    free(t);
    free(t2);
    free(Rci);

    client_clear(&client_test);

    return 0;
}
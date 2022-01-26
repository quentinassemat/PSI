#include "server.h"

// client et server font sur a.param
int server_init(server * server) {
    mpz_init_set_str(server->N, "179769313444375785804716951854212807170043487020564424359646658673359125268930341796385211192186165268958424981342683344803531250982610813881602937625776615479086270628175618202588941196085350840081586707369452911944254560327853117111162943558767673883041467956486674508460670039498430779083939927860702871553", 10);
    mpz_init_set_str(server->e, "59342290848241855381758259843496463780921957312662324296961624385704298102967", 10);
    mpz_init_set_str(server->d,"13884692741452720265309405451444615783675030644079234068970158493324801403133391673703132956997644225326040350053259308903620139192941934713959473854840110903201349907052645807168862206676744061022780389779556527755396513162348594513670236713110886315052442260025489531916329413297270985018260883315637408113", 10);

    // initialisation de data :
    profil_random(&server->data, RANDOM_DATA);

    hash_pair test = {4, 3};
    profil_append(&server->data, &test);
    hash_pair test2 = {6, 0};
    profil_append(&server->data, &test2);

    // initialisation de hashed :
    profil_hashed(&server->data);

    return  0;
}

int server_clear(server * server) {
    profil_clear(&server->data);
    return 0;
}

mpz_t * server_receive_round3(int fd, int * v) {  // pas oublier de faire le malloc pour y
    uchar * buffer = malloc(sizeof(uchar) * MPZ_BUF_SIZE); // on clean le malloc dans le main
    // on reçoit d'abord la taille (remplit v)
    fd_read(fd, buffer, MPZ_BUF_SIZE);
    sscanf((char*) buffer, "%d", v);
    // *v = atoi((char*) buffer);

    // printf("v is : %d\n", *v);

    // on reçoit les y en faisant un malloc
    mpz_t * y = (mpz_t *) malloc(sizeof(mpz_t) * (*v));
    for (int i = 0; i< *v; i++) {
        mpz_init(y[i]);
        memset(buffer, 0, MPZ_BUF_SIZE);
        fd_read(fd, buffer, MPZ_BUF_SIZE);
        // printf("buffer : %s\n", buffer);
        mpz_import(y[i], MPZ_BUF_SIZE, -1, 1, 0, 0, buffer);
    }
    free(buffer);
    return y;
}

int server_send_round5(int fd, int v, int w, mpz_t* y2, unsigned long * t) {
    uchar * buffer = malloc(sizeof(uchar) * MPZ_BUF_SIZE);
    
    // on envoie w
    sprintf((char*) buffer, "%d", w);
    fd_write(buffer, fd, MPZ_BUF_SIZE);

    // on envoie les y2
    for (int i = 0; i<v; i++) {
        memset(buffer, 0, MPZ_BUF_SIZE);
        mpz_export(buffer, NULL, -1, 1, 0, 0, y2[i]);
        // printf("buffer : %s\n", buffer);
        fd_write(buffer, fd, MPZ_BUF_SIZE); 
    }

    // on envoie les t 
    for (int i = 0; i<w; i++) {
        memset(buffer, 0, MPZ_BUF_SIZE);
        sprintf((char*) buffer, "%lu", t[i]);
        // printf("t_j : %lu\n", t[i]);
        fd_write(buffer, fd, MPZ_BUF_SIZE);
    }

    free(buffer);
    return 0;
}

int main(int argc, char* argv[]) {
    // initialisation des données du server
    server server_test;
    server_init(&server_test);
    // element_printf("generator : %B\n", server_test.g);
    printf("Data :\n");
    profil_print(stdout, &server_test.data);

    clock_t begin = clock();

    // Precomputation:
    printf("Precomputation :\n");
    unsigned long * t = (unsigned long *) malloc(sizeof(unsigned long) * server_test.data.tab->nb_elts);
    mpz_t Ksj, hsj;
    unsigned char data[MPZ_BUF_SIZE];
    unsigned char data_hashed[8];

    int index = 0;
    mpz_inits(Ksj, hsj, NULL);
    for (int j = 0; j<server_test.data.tab->size; j++) {
        if (hash_is_defined(server_test.data.tab, j)) {
            mpz_set_ui(hsj,(unsigned long int) server_test.data.tab->table[j].v);
            mpz_powm(Ksj, hsj, server_test.d, server_test.N);
            // gmp_printf("Ksj : %Zd\n", Ksj);
            memset(data, 0, MPZ_BUF_SIZE);
            memset(data_hashed, 0, 8);
            mpz_export(data, NULL, -1, 1, 0, 0, Ksj);
            // printf("data : %d\n", data[0]);
            sha3(data, MPZ_BUF_SIZE, data_hashed, 8);
            memcpy(&t[index], data_hashed, sizeof(unsigned long));
            // printf("t_j : %lu\n", t[index]);
            index += 1;
        }
    }
    mpz_clears(Ksj, hsj, NULL);


    clock_t end0 = clock();
    printf("temps exec precomputation (ms) = %f\n", ((double)(end0 - begin) / CLOCKS_PER_SEC) * 1000);


    // mise en place connection tcp
    printf("\n");
    int fd = socket_connect_server(PORT_SERVER);

    if (fd < 0) {
        return -1;
    }

    begin = clock();

    // Round 3
    printf("\n\nRound 3 :\n");

    int v;
    mpz_t * y = server_receive_round3(fd, &v); // tableau de taille v

    clock_t end1 = clock();
    printf("temps exec round 3 (ms) = %f\n", ((double)(end1 - begin) / CLOCKS_PER_SEC) * 1000);


    // Round 4
    printf("\n\nRound 4 :\n");

    // mpz_t * y2 = (mpz_t *) malloc(sizeof(mpz_t) * v);
    for (int i = 0; i<v; i++) {
        // mpz_init(y2[i]);
        // gmp_printf("y : %Zd\n", y[i]);
        mpz_powm(y[i], y[i], server_test.d, server_test.N);
        // gmp_printf("y2 : %Zd\n", y[i]);
    }

    clock_t end2 = clock();
    printf("temps exec round 4 (ms) = %f\n", ((double)(end2 - end1) / CLOCKS_PER_SEC) * 1000);


    // Round 5
    printf("\n\nRound 5 :\n");

    server_send_round5(fd, v, server_test.data.tab->nb_elts, y, t);

    clock_t end3 = clock();
    printf("temps exec round 5 (ms) = %f\n", ((double)(end3 - end2) / CLOCKS_PER_SEC) * 1000);

    close(fd);


    // clear des tableaux
    for (int i = 0; i<v; i++) {
        mpz_clear(y[i]);
        // mpz_clear(y2[i]);
    }
    free(y);
    // free(y2);
    free(t);
    server_clear(&server_test);
    return 0;
}

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
    profil_random(&server->data, 10);

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

element_t * server_receive_round2(int fd, int * v, element_t* X, pairing_t * pairing) {  // pas oublier de faire le malloc pour y
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    // on reçoit d'abord la taille (remplit v)
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    sscanf((char*) buffer, "%d", v);
    // *v = atoi((char*) buffer);

    printf("v is : %d\n", *v);

    // on reçoit X
    memset(buffer, 0, ELEMENT_BUF_SIZE);
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    element_from_bytes(*X, buffer);

    element_printf("X is : %B", *X);

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

int server_send_round4(int fd, int v, int w, element_t* Z, element_t* y2, unsigned long * t) {
    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    
    // on envoie w
    sprintf((char*) buffer, "%d", w);
    fd_write(buffer, fd, ELEMENT_BUF_SIZE);

    // on envoie Z
    memset(buffer, 0, ELEMENT_BUF_SIZE);
    int gen_size = element_to_bytes(buffer, *Z);
    if (gen_size < ELEMENT_BUF_SIZE) {
        return -1;
    }
    fd_write(buffer, fd, gen_size);

    // on envoie les y2
    for (int i = 0; i<v; i++) {
        memset(buffer, 0, ELEMENT_BUF_SIZE);
        gen_size = element_to_bytes(buffer, y2[i]);
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

    // mise en place connection tcp
    printf("\n");
    int fd = socket_connect_server(PORT_SERVER);

    if (fd < 0) {
        return -1;
    }

    // Round 1 (client)
    printf("\n\nRound 1 (client) :\n");

    // Round 2

    printf("\n\nRound 2 :\n");

    element_t X;
    element_init_G1(X, server_test.pairing);

    int v;

    element_t * y = server_receive_round2(fd, &v, &X, &server_test.pairing); // tableau de taille v


    // Round 3

    printf("\n\nRound 3 :\n");

    element_t Z, Rs, Ksj, hsj;
    element_init_G1(Z, server_test.pairing);
    element_init_Zr(Rs, server_test.pairing);
    element_init_G1(Ksj, server_test.pairing);
    element_init_G1(hsj, server_test.pairing);

    element_random(Rs);
    element_pow_zn(Z, server_test.g, Rs);

    // element_printf("Z is : %B\n", Z);

    unsigned char data[ELEMENT_BUF_SIZE];
    unsigned char data_hashed[8];
    int len;


    element_t * y2 = (element_t *) malloc(sizeof(element_t) * v);
    unsigned long * t = (unsigned long *) malloc(sizeof(unsigned long) * server_test.data.tab->nb_elts);

    int index = 0;
    for (int j = 0; j<server_test.data.tab->size; j++) {
        if (hash_is_defined(server_test.data.tab, j)) {
            element_from_hash(hsj , &server_test.data.tab->table[j].v, ELEMENT_BUF_SIZE);
            element_div(Ksj, X, hsj);
            element_pow_zn(Ksj, Ksj, Rs);
            len = element_to_bytes(data, Ksj);
            sha3(data, len, data_hashed, 8);
            memcpy(&t[index], data_hashed, sizeof(unsigned long));
            element_init_G1(y2[index], server_test.pairing);
            element_pow_zn(y2[index], y[index], Rs);
            index += 1;
        }
    }

    // Round 4

    printf("\n\nRound 4 :\n");


    server_send_round4(fd, v, server_test.data.tab->nb_elts, &Z, y2, t);

    // Round 5 (client)

    printf("\n\nRound 5 (client) :\n");

    close(fd);

    // clear des éléments
    element_clear(X);
    element_clear(Rs);
    element_clear(Z);
    element_clear(Ksj);
    element_clear(hsj);

    // clear des tableaux
    for (int i = 0; i<v; i++) {
        element_clear(y[i]);
        element_clear(y2[i]);
    }
    free(y);
    free(y2);
    free(t);
    server_clear(&server_test);
    return 0;
}

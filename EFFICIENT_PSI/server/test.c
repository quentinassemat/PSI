#include "test.h"

// test que la connexion marche bien entre le client et le server 
int test_tcp() {
    int fd = socket_connect_server(PORT_SERVER);
    char * buffer = malloc(sizeof(char) * BUF_SIZE);
    fd_read(fd, buffer, BUF_SIZE);
    free(buffer);
    close(fd);
    return 0;
}

// test que la structure de donnée profil, wrappant une table de hachage est fonctionnel, y compris la génération aléatoire de table
int test_profil() {
    profil_t pro;
    profil_init(&pro);
    hash_pair test = {4, 3};
    profil_append(&pro, &test);
    profil_print(stdout, &pro);
    profil_clear(&pro);
    profil_random(&pro, 5);
    profil_print(stdout, &pro);
    profil_into_file(&pro, "../sets/random.profil");
    profil_clear(&pro);
    return 0;
}

// test que la librairie pbc est bien installée et est fonctionnelle. 
int test_pbc() {
    // char test[360] = "type a\nq 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\nh 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\nr 730750818665451621361119245571504901405976559617\nexp2 159\nexp1 107\nsign1 1\nsign0 1";
    // initialisation du pairing (nécessaire pour initialiser des éléments d'une courbe elliptique)
    pbc_param_t par;
    element_t a, b, c;
    int rbits = 160;
    int qbits = 512;
    pbc_param_init_a_gen(par, rbits, qbits);
    pairing_t pairing;
    pairing_init_pbc_param(pairing, par);

    element_init_G1(a, pairing);
    element_init_G1(b, pairing);
    element_init_G1(c, pairing);

    element_random(a);
    element_set_si(a, 2);
    // element_out_str(stdout, 2, a);
    element_printf("a : %B, b : %B, c : %B\n", a, b, c);
    element_clear(a);
    element_clear(b);
    element_clear(c);

    pbc_param_clear(par);
    return 0;
}


// test que la structure de donnée profil, wrappant une table de hachage est fonctionnel, y compris la génération aléatoire de table
int test_hash() {
    profil_t pro;
    profil_init(&pro);
    hash_pair test = {4, 3};
    profil_append(&pro, &test);
    profil_print(stdout, &pro);
    profil_clear(&pro);
    profil_random(&pro, 5);
    profil_print(stdout, &pro);
    profil_into_file(&pro, "../sets/random.profil");
    profil_hashed(&pro);
    profil_print(stdout, &pro);
    // profil_into_file(&hash, "../sets/randomhashed.profil");
    // profil_clear(&hash);
    profil_clear(&pro);
    return 0;
}

int test_server() {
    int fd = socket_connect_server(PORT_SERVER);

    server server_test;
    server_init(&server_test);
    element_printf("generator : %B\n", server_test.g);
    printf("\nData :\n");
    profil_print(stdout, &server_test.data);

    uchar * buffer = malloc(sizeof(uchar) * ELEMENT_BUF_SIZE);
    fd_read(fd, buffer, ELEMENT_BUF_SIZE);
    element_t test, random;
    element_init_G1(test, server_test.pairing);
    element_init_G1(random, server_test.pairing);
    element_random(random);
    element_from_bytes(test, buffer);
    element_printf("test: %B\n", test);
    free(buffer);

    printf("Comparaison de test avec generator : %d\n", element_cmp(test, server_test.g));
    printf("Comparaison de d'un random avec gen : %d\n", element_cmp(test, random));
    
    close(fd);
    return 0;
}
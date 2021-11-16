#include "client.h"

// test que la connexion marche bien entre le client et le server 
int test_tcp() {
    int fd = socket_connect(PORT_SERVER);
    if (fd < 0) {
        return fd;
    }
    char * buffer = malloc(sizeof(char) * BUF_SIZE);
    strcpy(buffer, "Salut\0");
    fd_write(buffer, fd, BUF_SIZE);
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

int main(int argc, char* argv[]) {
    test_profil();
    return 0;
}
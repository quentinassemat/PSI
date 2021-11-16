#include "server.h"

int test_tcp() {
    int fd = socket_connect_server(PORT_SERVER);
    char * buffer = malloc(sizeof(char) * BUF_SIZE);
    fd_read(fd, buffer, BUF_SIZE);
    free(buffer);
    close(fd);
    return 0;
}

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

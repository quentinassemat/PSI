#include "client.h"

int main(int argc, char* argv[]) {
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
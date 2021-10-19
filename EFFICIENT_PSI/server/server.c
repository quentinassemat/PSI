#include "server.h"

int main(int argc, char* argv[]) {
    int fd = socket_connect_server(PORT_SERVER);
    char * buffer = malloc(sizeof(char) * BUF_SIZE);
    fd_read(fd, buffer, BUF_SIZE);
    free(buffer);
    close(fd);
}
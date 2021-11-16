#include "tcp.h"


int socket_connect_server(int port_number){
    //inititalisation of a TCP socket and sockaddr struct test
    printf("socket_connect_server:\n");
    unsigned int from_len = sizeof(struct sockaddr_in);
    int sock_fd = 0;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("listen socket created");

    struct sockaddr_in listen_socket;
    memset(&listen_socket, 0, sizeof(struct sockaddr_in));

    listen_socket.sin_family = AF_INET;
    listen_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_socket.sin_port = htons(port_number);

    //bind the socket
    printf("bind...");
    int bind_socket = bind(sock_fd, (struct sockaddr*)&listen_socket , from_len);
    if (bind_socket) {
        fprintf(stderr, "Could not bind socket: %s\n", strerror(errno));
        return -1;
    }
    printf("OK\n");
    printf("listen...");
    if (listen(sock_fd,10) == -1){
        fprintf(stderr, "Could not listen: %s\n", strerror(errno));
        return -1;
    };
    printf("OK\n");
    printf("accept...");
    int net_fd = accept(sock_fd, (struct sockaddr*)NULL ,NULL);
    if ((net_fd < 0)) {
      perror("accept()");
      exit(1);
    }
    printf("OK\n");
    printf("socket_connect_server succed\n\n");
    return net_fd;
}

int socket_connect(int port_number) {
    struct sockaddr_in dest;
    int mysocket = 0;
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mysocket<0){
        printf("(Fail in socket creation\n");
    }
    printf("Socket created\n");
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_ANY);
    dest.sin_port = htons(port_number);
    puts("(connect...");
    if (connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr))) {
        fprintf(stderr, "Could not connect: %s\n", strerror(errno));
        return 4;
    }
    printf("OK\n");
    printf("The socket is connected succes in socket_connect\n");
    return mysocket;
}

int fd_write(void * file, int fd, int size) {
    printf("write...");
    int bytes_writen = (int) write(fd, file, size);
    if (bytes_writen < 0){
        printf("FAIL error write\n");
        return -1;
    }
    printf("OK\n");
    printf("fd_write SUCCES\n");
    printf("message envoyé: %p\n",file);
    return bytes_writen;
}

int fd_read(int fd, void * file, int size) {
    printf("read...");
    int bytes_read = (int) read(fd, file, size);
    if (bytes_read < 0) {
        printf("FAIL error read\n");
        return -1;
    }
    printf("OK\n");
    printf("fd_read SUCCES\n");
    printf("message reçu: %p\n",file);
    return bytes_read;
}
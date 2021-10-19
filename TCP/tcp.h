#ifndef TCP_H
#define TCP_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define IP "127.0.0.1" // tout se passe en local
#define PORT_SERVER 5000

#define BUF_SIZE 1500


/**
 * This function creat and connect a socket to the port given on local interface
 * @param port_number the port number to listen to 
 * @return a socket descriptore
 */
int socket_connect_server(int port_number);

/**
 * This function creat and connect a socket to the port identified by port_number (everything is local)
 * @param port_number port number de la destination 
 * @return a socket descriptor
 * identified by (sc)
 */
int socket_connect(int port_number);

/**
 * This function send the file content to interface fd
 * identifié par (ts)
 * @param file a pointer to the file we want to send
 * @param fd file descriptore (can be a socket)
 * @param size size of the file
 * @return the number of bites sent if succed -1 if fail
 */
int fd_write(void * file, int fd, int size);

/**
 * This function listen and write the data from fd into file
 * @param file a pointer to the file we want to send
 * @param fd file descriptore (can be a socket)
 * @param size size of the file
 * @return the number of bites received if succed -1 if fail
 */
int fd_read(int fd, void * file, int size);





#endif // TCP_H

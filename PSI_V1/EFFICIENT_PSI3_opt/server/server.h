#ifndef SERVER_H
#define SERVER_H

#include "test.h"
#include "../../TCP/tcp.h"
#include "../../profil_opt_hach/profil.h"
#include "../../profil_opt_hach/buffer.h"
#include "../../../../pbc-0.5.14/include/pbc.h"
#include "../../../../pbc-0.5.14/include/pbc_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define generator_hash "0123456789"
#define RANDOM_DATA 5000
#define NONCE_SIZE 1

typedef struct server
{
    pairing_t pairing; // paramètre pour la bonne courbe elliptique
    element_t g; // générateur g commun au client et au server surla même courbe elliptique definié par pairing
    profil_t data; // correspond aux si du server
} server;

int server_init(server * server);

#endif // SERVER_H
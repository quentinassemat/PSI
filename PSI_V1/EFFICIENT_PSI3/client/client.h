#ifndef CLIENT_H
#define CLIENT_H

#include "test.h"
#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "../../profil/buffer.h"
#include "../../../../pbc-0.5.14/include/pbc.h"
#include "../../../../pbc-0.5.14/include/pbc_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define generator_hash "0123456789"
#define RANDOM_DATA 100
#define NONCE_SIZE 1

typedef struct client
{
    pairing_t pairing; // paramètre pour la bonne courbe elliptique
    element_t g; // générateur g commun au client et au server sur la même courbe elliptique definié par pairing
    profil_t data; // correspond aux ci du client
} client;

int client_init(client * client);

#endif // CLIENT_H
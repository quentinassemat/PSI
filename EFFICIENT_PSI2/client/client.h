#ifndef CLIENT_H
#define CLIENT_H

#include "test.h"
#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "../../profil/buffer.h"
#include "gmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// nombre de données
#define RANDOM_DATA 100

typedef struct client
{
    mpz_t N; 
    mpz_t e;
    profil_t data; // correspond aux ci du client
} client;

int client_init(client * client);

#endif // CLIENT_H
#ifndef SERVER_H
#define SERVER_H

#include "test.h"
#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "../../profil/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// nombre de données
#define RANDOM_DATA 5000

typedef struct server
{
    mpz_t N; 
    mpz_t d;
    mpz_t e;
    profil_t data; // correspond aux si du server
} server;

int server_init(server * server);

#endif // SERVER_H
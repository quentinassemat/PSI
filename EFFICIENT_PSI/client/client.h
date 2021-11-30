#ifndef CLIENT_H
#define CLIENT_H

#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "test.h"
#include "../../../../pbc-0.5.14/include/pbc.h"
#include "../../../../pbc-0.5.14/include/pbc_test.h"

typedef struct client
{
    pairing_t pairing; // paramètre pour la bonne courbe elliptique
    element_t g; // générateur g commun au client et au server sur la même courbe elliptique definié par pairing
    profil_t data; // correspond aux ci du client
    profil_t hashed; // correspond aux hci = H(ci) du client
    profil_t signatures; // correspond aux sigma_i = hci^d
} client;




#endif // CLIENT_H
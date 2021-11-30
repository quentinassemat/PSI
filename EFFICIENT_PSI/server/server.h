#ifndef SERVER_H
#define SERVER_H

#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "../../../../pbc-0.5.14/include/pbc.h"
#include "../../../../pbc-0.5.14/include/pbc_test.h"

struct server
{
    pairing_t pairing; // paramètre pour la bonne courbe elliptique
    element_t g; // générateur g commun au client et au server surla même courbe elliptique definié par pairing
    profil_t data; // correspond aux si du server
    profil_t hashed; // correspond aux hsi = H(si) du server
};

#endif // SERVER_H
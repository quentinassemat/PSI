#ifndef TEST_H
#define TEST_H

#include "server.h"
#include "../../TCP/tcp.h"
#include "../../profil/profil.h"
#include "../../../../pbc-0.5.14/include/pbc.h"
#include "../../../../pbc-0.5.14/include/pbc_test.h"

int test_tcp();
int test_profil();
int test_pbc();
int test_hash();
int test_server();

#endif // TEST_H
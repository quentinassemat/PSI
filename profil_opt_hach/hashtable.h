#ifndef HASHTABLE_H
#define HASHTABLE_H

/*! \file hashtable.h
 * \author Francois Morain morain@lix.polytechnique.fr
 * \date Last modification October 12, 2017                     
**************************************************************/

#define HASH_TABLE_FULL           -3
#define HASH_TABLE_ALREADY_EXISTS -2
#define HASH_FOUND                 1
#define HASH_NOT_FOUND             2

#define HASH_MODULI 1048583 // 2^20 + 7 > 2^15

#define ELEMENT_BUF_SIZE 128

typedef unsigned long hash_key;
// les hash value pointent vers un tableau de bytes qui représentent un point de courbe elliptique [u8; 128]
typedef unsigned char * hash_value;

// k représente un élément et v servira au "data transfer"
typedef struct{
    hash_key k;
    hash_value v;
} hash_pair;

typedef struct {
    int size;    /*!< number of pairs allocated in the table */
    int nb_elts; /*!< current number of pairs in the table */
    int modulo;  /*!< used for open addressing */
    hash_pair *table; /*!< the table storing pairs */
} hash_table_type, *hash_table;

#define hash_is_defined(H, addr) ((H)->table[(addr)].k != ULONG_MAX)

extern hash_table hash_init();
extern int hash_put(hash_table H, hash_key k, hash_value v);
extern int hash_get(hash_pair *kv, hash_table H, hash_key k);
extern void hash_clear(hash_table H);

#endif
#ifndef PROFIL_H
#define PROFIL_H

/*! \file profil.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include "hashtable.h"
#include "sha3.h"
#include "buffer.h"
#include "gmp.h"


/********** profils **********/

// profil contient les différents éléments d'un set
// on représente ces éléments par un ulong (un peu comme une clef primaire)

typedef unsigned long int ulong; // comme size_t mais bizarre d'avoir size pour les éléments

typedef struct{
    ulong id; // identifiant du joueur
    hash_table tab; // table de hachage des données du joueurs
} profil_t;

//création et gestion de la mémoire

void profil_init(profil_t *pro);
void profil_clear(profil_t *pro);
void profil_reset(profil_t * pro);

void profil_append(profil_t *pro, hash_pair * hp);

int profil_from_file(profil_t *buf, const char *file_name);
void profil_random(profil_t *out, unsigned long nb_elements);
int profil_into_file(profil_t *pro, const char *file_name);
int profil_hashed(profil_t * in, profil_t * out);

void profil_print(FILE *ofile, profil_t *pro);


#endif
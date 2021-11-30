#include "profil.h"

void profil_init(profil_t *pro)
{
    srand(time(NULL));
    pro->tab = hash_init(ULONG_MAX);
    pro->id = rand() % ULONG_MAX;
}

void profil_clear(profil_t *pro)
{
    hash_clear(pro->tab);
}

void profil_reset(profil_t * pro) {
    hash_clear(pro->tab);
    pro->tab = hash_init(ULONG_MAX);
    pro->id = rand() % ULONG_MAX;
}

void profil_append(profil_t *pro, hash_pair * hp)
{
    hash_put(pro->tab, hp->k, hp->v);
}

int profil_from_file(profil_t *pro, const char *file_name)
{
    profil_reset(pro);
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        return 0;
    }
    unsigned long k, v;
    fscanf(file, "%lu\n", &pro->id);
    while (!feof(file))
    {
        fscanf(file, "%lu %lu\n", &k, &v);
        hash_pair hp = {k, v};
        profil_append(pro, &hp);
    }
    fclose(file);
    return 1;
}

void profil_print(FILE *fd, profil_t *pro)
{
    fprintf(fd, "profil :\n");
    fprintf(fd, "%lu\n", pro->id);
    for (size_t i = 0; i < pro->tab->size ; i++)
    {
        if (hash_is_defined(pro->tab, i)) {
            fprintf(fd, "%lu %lu\n", pro->tab->table[i].k, pro->tab->table[i].v);
        }
    }
}

int profil_into_file(profil_t *pro, const char *file_name) {
    FILE *file = fopen(file_name, "w");
    if (file == NULL)
    {
        return 0;
    }
    fprintf(file, "%lu\n", pro->id);
    for (size_t i = 0; i < pro->tab->size ; i++)
    {
        if (hash_is_defined(pro->tab, i)) {
            fprintf(file, "%lu %lu\n", pro->tab->table[i].k, pro->tab->table[i].v);
        }
    }
    fclose(file);
    return 1;
}

int profil_hashed(profil_t * in, profil_t * out) {
    profil_init(out);
    for (size_t i = 0; i<in->tab->size; i++) {
        if (hash_is_defined(in->tab, i)) {
            buffer_t buf_to_hash, buf_hashed;
            buffer_init(&buf_to_hash, 8);
            buffer_init(&buf_hashed, 8);
            for (int i = 0; i<8; i++) {
                uchar c = in->tab->table[i].k >> 4 * i;
                buffer_append_uchar(&buf_to_hash, c);
            }
            buffer_hash(&buf_hashed, 8, &buf_to_hash);
            mpz_t export;
            mpz_init(export);
            mpz_import(export, buf_hashed.length, 1, 1, 1, 0, buf_hashed.tab);
            hash_pair hp = {mpz_get_ui(export), in->tab->table[i].v};
            profil_append(out, &hp);
        }
    }
    return 0;
}

// juste le remplissage de la table de hachage
void profil_random(profil_t * out, unsigned long nb_elements) {
    profil_init(out);
    for (size_t i = 0; i<nb_elements; i++) {
        hash_pair hp = {rand() % ULONG_MAX, rand() % ULONG_MAX};
        profil_append(out, &hp);
    }
}

// finir profil
// tester création aléatoire
// tester création depuis fichier
// mettre en place hachage
// mettre en place courbe elliptique
// faire un semblant de protocole
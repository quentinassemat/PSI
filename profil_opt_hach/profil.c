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

void profil_print(FILE *fd, profil_t *pro, pairing_t pairing)
{   
    element_t print;
    element_init_G1(print, pairing);
    fprintf(fd, "profil 2 :\n");
    fprintf(fd, "%lu\n", pro->id);
    for (size_t i = 0; i < pro->tab->size ; i++)
    {
        if (hash_is_defined(pro->tab, i)) {
            element_from_bytes(print, pro->tab->table[i].v);
            element_printf("%lu %B\n", pro->tab->table[i].k, print);
            // fprintf(fd, "%lu %s\n", pro->tab->table[i].k, pro->tab->table[i].v);
        }
    }
}
// ((unsigned char *)pro->tab->table[i].v)[0],  ((unsigned char *)pro->tab->table[i].v)[1]

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
            fprintf(file, "%lu %hhn\n", pro->tab->table[i].k, pro->tab->table[i].v);
        }
    }
    fclose(file);
    return 1;
}

int profil_hashed(profil_t * in, pairing_t pairing) {
    // profil_init(out);
    buffer_t buf_to_hash, buf_hashed;
    for (size_t i = 0; i<in->tab->size; i++) {
        if (hash_is_defined(in->tab, i)) {
            buffer_init(&buf_to_hash, 8);
            buffer_init(&buf_hashed, 8);
            for (int j = 0; j<4; j++) {
                uchar c = in->tab->table[i].k >> (8 * j);
                buffer_append_uchar(&buf_to_hash, c);
            }
            buffer_hash(&buf_hashed, 8, &buf_to_hash);
            element_t from_hash;
            element_init_G1(from_hash, pairing);
            element_from_hash(from_hash, buf_hashed.tab, 8);
            in->tab->table[i].v = (hash_value) malloc(128 * sizeof(char));
            element_to_bytes(in->tab->table[i].v, from_hash);
            buffer_clear(&buf_to_hash);
            buffer_clear(&buf_hashed);
        }
    }
    return 0;
}

// juste le remplissage de la table de hachage
void profil_random(profil_t * out, unsigned long nb_elements) {
    profil_init(out);
    for (size_t i = 0; i<nb_elements; i++) {
        // hash_pair hp = {rand() % ULONG_MAX, rand() % ULONG_MAX};
        hash_pair hp = {rand() % ULONG_MAX, NULL};
        profil_append(out, &hp);
    }
}
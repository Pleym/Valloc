#ifndef VALLOC_H
#define VALLOC_H

#include <stddef.h>

// Structure pour stocker les métadonnées de chaque bloc
typedef struct block_header {
    size_t size;           // Taille du bloc (sans compter l'en-tête)
    int is_free;          // 1 si le bloc est libre, 0 sinon
} block_header_t;

// Fonctions principales de l'allocateur
void* my_malloc(size_t size);
void my_free(void* ptr);

#endif // VALLOC_H

#include "valloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

// Implémentation basique de my_malloc utilisant mmap
void* my_malloc(size_t size) {
    if (size == 0) return NULL;

    // Calculer la taille totale nécessaire (en-tête + données)
    size_t total_size = size + sizeof(block_header_t);
    
    // Arrondir à la taille de page supérieure
    size_t page_size = (size_t)getpagesize();
    size_t aligned_size = (total_size + page_size - 1) & ~(page_size - 1);

    // Allouer la mémoire avec mmap
    void* ptr = mmap(NULL, aligned_size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);

    if (ptr == MAP_FAILED) {
        return NULL;
    }

    // Initialiser l'en-tête du bloc
    block_header_t* header = (block_header_t*)ptr;
    header->size = aligned_size - sizeof(block_header_t);
    header->is_free = 0;

    // Retourner le pointeur vers la zone de données (après l'en-tête)
    return (void*)((char*)ptr + sizeof(block_header_t));
}

// Implémentation basique de my_free utilisant munmap
void my_free(void* ptr) {
    if (!ptr) return;

    // Récupérer l'en-tête du bloc
    block_header_t* header = (block_header_t*)((char*)ptr - sizeof(block_header_t));
    size_t total_size = header->size + sizeof(block_header_t);

    // Libérer la mémoire avec munmap
    if (munmap(header, total_size) == -1) {
        fprintf(stderr, "Error in my_free: munmap failed with errno %d\n", errno);
    }
}

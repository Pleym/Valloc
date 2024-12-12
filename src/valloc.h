#ifndef VALLOC_H
#define VALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

// Structures de données
typedef struct MemoryBlock {
    void* adress;
    size_t size;
    bool status;
} MemoryBlock;

typedef struct MemoryAllocator {
    MemoryBlock* blocks;
    size_t total_blocks;
    size_t used_blocks;
    pthread_mutex_t mutex;  // Mutex pour protéger l'accès concurrent
    bool initialized;       // Flag pour vérifier l'initialisation
} MemoryAllocator;

// Fonctions d'allocation
void* valloc(size_t size);
void vfree(void* ptr, size_t size);

// Fonctions de gestion de l'allocateur
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks);
void valloc_destroy(MemoryAllocator* allocator);
void* valloc_block(MemoryAllocator* allocator, size_t size);
void revalloc(MemoryAllocator* allocator, void* ptr);

#endif // VALLOC_H

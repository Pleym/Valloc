#include <sys/mman.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "valloc.h"

// Initialisation de l'allocateur
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks) {
    if (allocator == NULL || initial_blocks == 0) {
        return -1;
    }

    allocator->blocks = malloc(initial_blocks * sizeof(MemoryBlock));
    if (allocator->blocks == NULL) {
        return -1;
    }

    // Initialisation des blocs
    for (size_t i = 0; i < initial_blocks; i++) {
        allocator->blocks[i].adress = NULL;
        allocator->blocks[i].size = 0;
        allocator->blocks[i].status = true;
    }

    // Initialisation du mutex
    if (pthread_mutex_init(&allocator->mutex, NULL) != 0) {
        free(allocator->blocks);
        return -1;
    }

    allocator->total_blocks = initial_blocks;
    allocator->used_blocks = 0;
    allocator->initialized = true;

    return 0;
}

// Destruction de l'allocateur
void valloc_destroy(MemoryAllocator* allocator) {
    if (allocator == NULL || !allocator->initialized) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);

    // Libération des blocs encore alloués
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (!allocator->blocks[i].status && allocator->blocks[i].adress != NULL) {
            vfree(allocator->blocks[i].adress, allocator->blocks[i].size);
        }
    }

    free(allocator->blocks);
    allocator->blocks = NULL;
    allocator->total_blocks = 0;
    allocator->used_blocks = 0;

    pthread_mutex_unlock(&allocator->mutex);
    pthread_mutex_destroy(&allocator->mutex);
    allocator->initialized = false;
}

// Allocation simple avec mmap
void* valloc(size_t size) {
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("valloc failed");
        return NULL;
    }
    return ptr;
}

// Libération de la mémoire
void vfree(void* ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        if (munmap(ptr, size) == -1) {
            perror("vfree failed");
        }
    }
}

// Allocation d'un bloc avec gestion du mutex
void* valloc_block(MemoryAllocator* allocator, size_t size) {
    if (allocator == NULL || !allocator->initialized || size == 0) {
        return NULL;
    }

    void* result = NULL;
    pthread_mutex_lock(&allocator->mutex);

    // Recherche d'un bloc libre
    for (size_t i = 0; i < allocator->total_blocks; ++i) {
        if (allocator->blocks[i].status && allocator->blocks[i].size >= size) {
            allocator->blocks[i].status = false;
            allocator->used_blocks++;
            result = allocator->blocks[i].adress;
            pthread_mutex_unlock(&allocator->mutex);
            return result;
        }
    }

    // Allocation d'un nouveau bloc
    void* new_block = valloc(size);
    if (new_block == NULL) {
        pthread_mutex_unlock(&allocator->mutex);
        return NULL;
    }

    // Ajout du nouveau bloc dans la liste
    if (allocator->used_blocks < allocator->total_blocks) {
        size_t index = allocator->used_blocks;
        allocator->blocks[index].adress = new_block;
        allocator->blocks[index].size = size;
        allocator->blocks[index].status = false;
        allocator->used_blocks++;
        result = new_block;
    } else {
        vfree(new_block, size);
    }

    pthread_mutex_unlock(&allocator->mutex);
    return result;
}

// Libération d'un bloc avec gestion du mutex
void revalloc(MemoryAllocator* allocator, void* ptr) {
    if (allocator == NULL || !allocator->initialized || ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);

    for (size_t i = 0; i < allocator->total_blocks; ++i) {
        if (allocator->blocks[i].adress == ptr) {
            allocator->blocks[i].status = true;
            allocator->used_blocks--;
            break;
        }
    }

    pthread_mutex_unlock(&allocator->mutex);
}

// Macro pour les tests
#define VALLOC_BLOC(TYPE, nbr_element) do { \
    size_t size = (size_t)(nbr_element) * sizeof(TYPE); \
    void* ptr = valloc(size); \
    if (ptr == NULL) { \
        fprintf(stderr, "Allocation failed\n"); \
        return 1; \
    } \
    printf("Allocated %zu bytes for %zu elements of type " #TYPE "\n", size, (size_t)(nbr_element)); \
    vfree(ptr, size); \
    printf("Successfully freed %zu bytes\n", size); \
} while(0)

#ifndef VALLOC_H
#define VALLOC_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_CACHE_BLOCKS 32
#define MAX_THREADS 16

// Structure pour un bloc de cache
typedef struct CacheBlock {
    void* ptr;
    size_t size;
} CacheBlock;

// Cache par thread
typedef struct ThreadCache {
    CacheBlock blocks[MAX_CACHE_BLOCKS];
    int count;
    pthread_mutex_t mutex;
} ThreadCache;

// Structure pour un bloc de mémoire
typedef struct MemoryBlock {
    void* adress;
    size_t size;
    bool status;  // true = libre, false = occupé
    bool recycled;  // true si le bloc est dans le cache de recyclage
} MemoryBlock;

// Structure principale de l'allocateur
typedef struct {
    MemoryBlock* blocks;
    size_t total_blocks;
    size_t used_blocks;
    size_t recycled_blocks;  // Nombre de blocs dans le cache de recyclage
    pthread_mutex_t mutex;
    bool initialized;
    ThreadCache thread_caches[MAX_THREADS];
    int num_threads;
} MemoryAllocator;

// Fonctions principales
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks, int num_threads);
void* valloc_block(MemoryAllocator* allocator, size_t size);
void revalloc(MemoryAllocator* allocator, void* ptr);
void free_valloc(MemoryAllocator* allocator, void* ptr);
void valloc_cleanup(MemoryAllocator* allocator);
void valloc_destroy(MemoryAllocator* allocator);

// Fonctions de cache
ThreadCache* get_thread_cache(MemoryAllocator* allocator);
void* cache_allocate(ThreadCache* cache, size_t size);
void cache_free(ThreadCache* cache, void* ptr, size_t size);

#endif

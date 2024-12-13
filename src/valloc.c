#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include "valloc.h"

// Variable thread-local pour stocker l'ID du thread
static __thread int thread_id = -1;
static int next_thread_id = 0;
static pthread_mutex_t thread_id_mutex = PTHREAD_MUTEX_INITIALIZER;

// Obtenir l'ID du thread actuel
static int get_thread_id() {
    if (thread_id == -1) {
        pthread_mutex_lock(&thread_id_mutex);
        thread_id = next_thread_id++;
        pthread_mutex_unlock(&thread_id_mutex);
    }
    return thread_id;
}

// Obtenir le cache du thread actuel
ThreadCache* get_thread_cache(MemoryAllocator* allocator) {
    int id = get_thread_id();
    if (id >= allocator->num_threads) return NULL;
    return &allocator->thread_caches[id];
}

// Allouer depuis le cache
void* cache_allocate(ThreadCache* cache, size_t size) {
    if (!cache) return NULL;

    pthread_mutex_lock(&cache->mutex);
    
    // Recherche d'un bloc de taille appropriée dans le cache
    for (int i = 0; i < cache->count; i++) {
        if (cache->blocks[i].size == size) {
            void* ptr = cache->blocks[i].ptr;
            // Déplacer le dernier bloc à cette position
            cache->blocks[i] = cache->blocks[--cache->count];
            pthread_mutex_unlock(&cache->mutex);
            return ptr;
        }
    }
    
    pthread_mutex_unlock(&cache->mutex);
    return NULL;
}

// Libérer dans le cache
void cache_free(ThreadCache* cache, void* ptr, size_t size) {
    if (!cache || !ptr) return;

    pthread_mutex_lock(&cache->mutex);
    
    if (cache->count < MAX_CACHE_BLOCKS) {
        cache->blocks[cache->count].ptr = ptr;
        cache->blocks[cache->count].size = size;
        cache->count++;
        pthread_mutex_unlock(&cache->mutex);
        return;
    }
    
    pthread_mutex_unlock(&cache->mutex);
    munmap(ptr, size);
}

// Initialisation de l'allocateur
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks, int num_threads) {
    if (allocator == NULL || initial_blocks == 0 || num_threads <= 0 || num_threads > MAX_THREADS) {
        return -1;
    }

    allocator->blocks = (MemoryBlock*)malloc(initial_blocks * sizeof(MemoryBlock));
    if (allocator->blocks == NULL) {
        return -1;
    }

    // Initialisation des blocs
    for (size_t i = 0; i < initial_blocks; i++) {
        allocator->blocks[i].adress = NULL;
        allocator->blocks[i].size = 0;
        allocator->blocks[i].status = true;  // true = libre
        allocator->blocks[i].recycled = false;
    }

    if (pthread_mutex_init(&allocator->mutex, NULL) != 0) {
        free(allocator->blocks);
        return -1;
    }

    allocator->total_blocks = initial_blocks;
    allocator->used_blocks = 0;
    allocator->recycled_blocks = 0;
    allocator->num_threads = num_threads;
    allocator->initialized = true;

    // Initialisation des caches par thread
    for (int i = 0; i < num_threads; i++) {
        if (pthread_mutex_init(&allocator->thread_caches[i].mutex, NULL) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_mutex_destroy(&allocator->thread_caches[j].mutex);
            }
            pthread_mutex_destroy(&allocator->mutex);
            free(allocator->blocks);
            return -1;
        }
        allocator->thread_caches[i].count = 0;
    }

    return 0;
}

// Allocation de mémoire
void* valloc_block(MemoryAllocator* allocator, size_t size) {
    if (allocator == NULL || !allocator->initialized || size == 0) {
        return NULL;
    }

    // Essayer d'abord le cache
    ThreadCache* cache = get_thread_cache(allocator);
    if (cache) {
        void* ptr = cache_allocate(cache, size);
        if (ptr) return ptr;
    }

    pthread_mutex_lock(&allocator->mutex);

    // Chercher d'abord un bloc recyclé de taille appropriée
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].status && allocator->blocks[i].recycled && 
            allocator->blocks[i].size >= size) {
            allocator->blocks[i].status = false;
            allocator->blocks[i].recycled = false;
            allocator->used_blocks++;
            allocator->recycled_blocks--;
            void* ptr = allocator->blocks[i].adress;
            pthread_mutex_unlock(&allocator->mutex);
            return ptr;
        }
    }

    // Si pas de bloc recyclé disponible, allouer un nouveau bloc
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        pthread_mutex_unlock(&allocator->mutex);
        return NULL;
    }

    // Chercher un emplacement libre dans les blocs
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].status && !allocator->blocks[i].recycled) {
            allocator->blocks[i].adress = ptr;
            allocator->blocks[i].size = size;
            allocator->blocks[i].status = false;
            allocator->blocks[i].recycled = false;
            allocator->used_blocks++;
            pthread_mutex_unlock(&allocator->mutex);
            return ptr;
        }
    }

    // Si aucun emplacement libre n'est trouvé
    munmap(ptr, size);
    pthread_mutex_unlock(&allocator->mutex);
    return NULL;
}

// Libération pure de mémoire
void free_valloc(MemoryAllocator* allocator, void* ptr) {
    if (allocator == NULL || !allocator->initialized || ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);

    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].adress == ptr) {
            size_t size = allocator->blocks[i].size;
            
            // Essayer d'abord de mettre dans le cache
            ThreadCache* cache = get_thread_cache(allocator);
            if (cache) {
                cache_free(cache, ptr, size);
            } else {
                munmap(ptr, size);
            }

            allocator->blocks[i].status = true;  // Marquer comme libre
            allocator->blocks[i].adress = NULL;
            allocator->blocks[i].recycled = false;
            allocator->used_blocks--;
            pthread_mutex_unlock(&allocator->mutex);
            return;
        }
    }

    pthread_mutex_unlock(&allocator->mutex);
}

// Recyclage de mémoire
void revalloc(MemoryAllocator* allocator, void* ptr) {
    if (allocator == NULL || !allocator->initialized || ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);

    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].adress == ptr) {
            // Marquer le bloc comme libre et recyclé
            allocator->blocks[i].status = true;
            allocator->blocks[i].recycled = true;
            allocator->used_blocks--;
            allocator->recycled_blocks++;
            pthread_mutex_unlock(&allocator->mutex);
            return;
        }
    }

    pthread_mutex_unlock(&allocator->mutex);
}

// Nettoyage de l'allocateur
void valloc_cleanup(MemoryAllocator* allocator) {
    if (!allocator || !allocator->initialized) return;

    // Libérer tous les caches
    for (int i = 0; i < allocator->num_threads; i++) {
        ThreadCache* cache = &allocator->thread_caches[i];
        pthread_mutex_lock(&cache->mutex);
        for (int j = 0; j < cache->count; j++) {
            munmap(cache->blocks[j].ptr, cache->blocks[j].size);
        }
        pthread_mutex_unlock(&cache->mutex);
        pthread_mutex_destroy(&cache->mutex);
    }

    // Libérer la mémoire principale et les blocs recyclés
    pthread_mutex_lock(&allocator->mutex);
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if ((!allocator->blocks[i].status || allocator->blocks[i].recycled) && 
            allocator->blocks[i].adress != NULL) {
            munmap(allocator->blocks[i].adress, allocator->blocks[i].size);
        }
    }
    free(allocator->blocks);
    pthread_mutex_unlock(&allocator->mutex);
    pthread_mutex_destroy(&allocator->mutex);
    allocator->initialized = false;
}

// Destruction complète
void valloc_destroy(MemoryAllocator* allocator) {
    valloc_cleanup(allocator);
    memset(allocator, 0, sizeof(MemoryAllocator));
}

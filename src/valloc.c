#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include "valloc.h"

// Variable thread-local pour stocker l'ID du thread
__thread int thread_id = -1;
int next_thread_id = 0;
pthread_mutex_t thread_id_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Obtient l'ID du thread courant
 * 
 * Initialise et retourne un identifiant unique de thread de manière paresseuse.
 * Thread-safe grâce à la protection par mutex.
 * 
 * @return int Identifiant unique du thread
 */
int get_thread_id() {
    if (thread_id == -1) {
        pthread_mutex_lock(&thread_id_mutex);
        thread_id = next_thread_id++;
        pthread_mutex_unlock(&thread_id_mutex);
    }
    return thread_id;
}

/**
 * @brief Récupère le cache thread-local pour le thread courant
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @return ThreadCache* Pointeur vers le cache du thread, NULL si l'ID est invalide
 */
ThreadCache* get_thread_cache(MemoryAllocator* allocator) {
    int id = get_thread_id();
    if (id >= allocator->num_threads) return NULL;
    return &allocator->thread_caches[id];
}

/**
 * @brief Tente d'allouer de la mémoire depuis le cache thread-local
 * 
 * Recherche dans le cache du thread un bloc de la taille demandée.
 * Thread-safe grâce au mutex du cache.
 * 
 * @param cache Cache du thread pour l'allocation
 * @param size Taille du bloc de mémoire nécessaire
 * @return void* Pointeur vers le bloc en cache, NULL si aucun bloc approprié trouvé
 */
void* cache_allocate(ThreadCache* cache, size_t size) {
    if (!cache) return NULL;

    pthread_mutex_lock(&cache->mutex);
    for (int i = 0; i < cache->count; i++) {
        if (cache->blocks[i].size == size) {
            void* ptr = cache->blocks[i].ptr;
            cache->blocks[i] = cache->blocks[--cache->count];
            pthread_mutex_unlock(&cache->mutex);
            return ptr;
        }
    }
    pthread_mutex_unlock(&cache->mutex);
    return NULL;
}

/**
 * @brief Tente de mettre en cache un bloc de mémoire libéré
 * 
 * Si le cache est plein, la mémoire est retournée au système.
 * Thread-safe grâce au mutex du cache.
 * 
 * @param cache Cache du thread pour le stockage
 * @param ptr Pointeur vers le bloc de mémoire
 * @param size Taille du bloc de mémoire
 */
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

/**
 * @brief Initialise l'allocateur de mémoire
 * 
 * Alloue le pool global de mémoire et initialise tous les blocs.
 * Initialise le mutex global et les caches des threads.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param initial_blocks Nombre initial de blocs dans le pool
 * @param num_threads Nombre de threads à supporter
 * @return int 0 en cas de succès, -1 en cas d'échec
 */
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks, int num_threads) {
    if (allocator == NULL || initial_blocks == 0 || num_threads <= 0 || num_threads > MAX_THREADS) {
        return -1;
    }

    // Allocation du pool global de mémoire
    allocator->blocks = (MemoryBlock*)malloc(initial_blocks * sizeof(MemoryBlock));
    if (allocator->blocks == NULL) {
        return -1;
    }

    // Initialisation de tous les blocs dans le pool
    for (size_t i = 0; i < initial_blocks; i++) {
        allocator->blocks[i].adress = NULL;
        allocator->blocks[i].size = 0;
        allocator->blocks[i].status = true;
        allocator->blocks[i].recycled = false;
    }

    // Initialisation du mutex global
    if (pthread_mutex_init(&allocator->mutex, NULL) != 0) {
        free(allocator->blocks);
        return -1;
    }

    // Initialisation de l'état de l'allocateur
    allocator->total_blocks = initial_blocks;
    allocator->used_blocks = 0;
    allocator->recycled_blocks = 0;
    allocator->num_threads = num_threads;
    allocator->initialized = true;

    // Initialisation des caches des threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_mutex_init(&allocator->thread_caches[i].mutex, NULL) != 0) {
            // Nettoyage en cas d'échec
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

/**
 * @brief Alloue un bloc de mémoire
 * 
 * Tente d'abord d'allouer depuis le cache thread-local.
 * Si l'allocation du cache échoue, recherche un bloc recyclé dans le pool global.
 * Si aucun bloc recyclé n'est disponible, alloue de la nouvelle mémoire.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param size Taille du bloc de mémoire nécessaire
 * @return void* Pointeur vers le bloc de mémoire alloué, NULL en cas d'échec
 */
void* valloc_block(MemoryAllocator* allocator, size_t size) {
    if (allocator == NULL || !allocator->initialized || size == 0) {
        return NULL;
    }

    // Chemin rapide : essai du cache thread-local d'abord
    ThreadCache* cache = get_thread_cache(allocator);
    if (cache) {
        void* ptr = cache_allocate(cache, size);
        if (ptr) return ptr;
    }

    // Chemin lent : accès au pool global
    pthread_mutex_lock(&allocator->mutex);

    // Recherche d'abord un bloc recyclé
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

    // Allocation de nouvelle mémoire si aucun bloc recyclé disponible
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        pthread_mutex_unlock(&allocator->mutex);
        return NULL;
    }

    // Recherche d'un emplacement libre dans le tableau de blocs
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

    // Aucun emplacement libre disponible
    munmap(ptr, size);
    pthread_mutex_unlock(&allocator->mutex);
    return NULL;
}

/**
 * @brief Libère un bloc de mémoire
 * 
 * Tente d'abord de mettre le bloc en cache.
 * Si la mise en cache échoue, retourne le bloc au système.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param ptr Pointeur vers le bloc de mémoire à libérer
 */
void free_valloc(MemoryAllocator* allocator, void* ptr) {
    if (allocator == NULL || !allocator->initialized || ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);

    // Recherche du bloc dans le pool global
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].adress == ptr) {
            size_t size = allocator->blocks[i].size;
            
            // Tente d'abord de mettre en cache le bloc
            ThreadCache* cache = get_thread_cache(allocator);
            if (cache) {
                cache_free(cache, ptr, size);
            } else {
                munmap(ptr, size);
            }

            // Mise à jour du statut du bloc
            allocator->blocks[i].status = true;
            allocator->blocks[i].adress = NULL;
            allocator->blocks[i].recycled = false;
            allocator->used_blocks--;
            pthread_mutex_unlock(&allocator->mutex);
            return;
        }
    }

    pthread_mutex_unlock(&allocator->mutex);
}

/**
 * @brief Recycle un bloc de mémoire
 * 
 * Marque le bloc comme recyclé et disponible pour réutilisation.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param ptr Pointeur vers le bloc de mémoire à recycler
 */
void revalloc(MemoryAllocator* allocator, void* ptr) {
    if (allocator == NULL || !allocator->initialized || ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);


    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].adress == ptr) {
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

/**
 * @brief Nettoie l'allocateur
 * 
 * Libère tous les blocs recyclés et les caches des threads.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 */
void valloc_cleanup(MemoryAllocator* allocator) {
    if (allocator == NULL || !allocator->initialized) {
        return;
    }

    pthread_mutex_lock(&allocator->mutex);


    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (allocator->blocks[i].recycled) {
            munmap(allocator->blocks[i].adress, allocator->blocks[i].size);
            allocator->blocks[i].adress = NULL;
            allocator->blocks[i].size = 0;
            allocator->blocks[i].status = true;
            allocator->blocks[i].recycled = false;
            allocator->recycled_blocks--;
        }
    }

    pthread_mutex_unlock(&allocator->mutex);
}

/**
 * @brief Détruit l'allocateur
 * 
 * Appelle valloc_cleanup puis libère toute la mémoire restante.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 */
void valloc_destroy(MemoryAllocator* allocator) {
    if (allocator == NULL || !allocator->initialized) {
        return;
    }

    valloc_cleanup(allocator);
    

    for (int i = 0; i < allocator->num_threads; i++) {
        ThreadCache* cache = &allocator->thread_caches[i];
        pthread_mutex_lock(&cache->mutex);
        

        for (int j = 0; j < cache->count; j++) {
            munmap(cache->blocks[j].ptr, cache->blocks[j].size);
        }
        cache->count = 0;
        
        pthread_mutex_unlock(&cache->mutex);
        pthread_mutex_destroy(&cache->mutex);
    }


    pthread_mutex_lock(&allocator->mutex);
    for (size_t i = 0; i < allocator->total_blocks; i++) {
        if (!allocator->blocks[i].status) {
            munmap(allocator->blocks[i].adress, allocator->blocks[i].size);
        }
    }
    
    free(allocator->blocks);
    pthread_mutex_unlock(&allocator->mutex);
    pthread_mutex_destroy(&allocator->mutex);
    
    allocator->initialized = false;
}

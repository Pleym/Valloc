#ifndef VALLOC_H
#define VALLOC_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

// Nombre maximum de blocs pouvant être mis en cache par thread
#define MAX_CACHE_BLOCKS 32
// Nombre maximum de threads supportés par l'allocateur
#define MAX_THREADS 16

/**
 * @brief Structure d'un bloc de cache thread-local
 * 
 * Représente un bloc unique dans le cache thread-local.
 * Utilisé pour stocker les blocs de mémoire récemment libérés
 * pour une réutilisation rapide.
 */
typedef struct CacheBlock {
    void* ptr;          // Pointeur vers le bloc de mémoire
    size_t size;        // Taille du bloc de mémoire
} CacheBlock;

/**
 * @brief Structure du cache thread-local
 * 
 * Chaque thread maintient son propre cache de blocs de mémoire
 * récemment libérés pour réduire la contention et améliorer
 * la vitesse d'allocation.
 */
typedef struct ThreadCache {
    CacheBlock blocks[MAX_CACHE_BLOCKS];  // Tableau des blocs en cache
    int count;                            // Nombre de blocs actuellement en cache
    pthread_mutex_t mutex;                // Mutex pour les opérations thread-safe
} ThreadCache;

/**
 * @brief Structure des métadonnées d'un bloc de mémoire
 * 
 * Suit l'état et les propriétés de chaque bloc de mémoire
 * géré par l'allocateur.
 */
typedef struct MemoryBlock {
    void* adress;       // Adresse du bloc de mémoire alloué
    size_t size;        // Taille du bloc de mémoire
    bool status;        // true = libre, false = occupé
    bool recycled;      // true si le bloc est dans le cache de recyclage
} MemoryBlock;

/**
 * @brief Structure principale de l'allocateur de mémoire
 * 
 * Structure centrale qui gère le système d'allocation mémoire.
 * Gère à la fois le pool global de mémoire et les caches thread-locaux.
 */
typedef struct {
    MemoryBlock* blocks;                    // Tableau de tous les blocs (pool global)
    size_t total_blocks;                    // Nombre total de blocs dans le pool
    size_t used_blocks;                     // Nombre de blocs actuellement utilisés
    size_t recycled_blocks;                 // Nombre de blocs dans le cache de recyclage
    pthread_mutex_t mutex;                  // Mutex global pour les opérations sur le pool
    bool initialized;                       // État d'initialisation
    ThreadCache thread_caches[MAX_THREADS]; // Tableau des caches thread-locaux
    int num_threads;                        // Nombre de threads actifs
} MemoryAllocator;

/**
 * @brief Initialise l'allocateur de mémoire
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param initial_blocks Taille initiale du pool de mémoire
 * @param num_threads Nombre de threads à supporter
 * @return int 0 en cas de succès, -1 en cas d'échec
 */
int valloc_init(MemoryAllocator* allocator, size_t initial_blocks, int num_threads);

/**
 * @brief Alloue un bloc de mémoire
 * 
 * Tente d'abord d'allouer depuis le cache thread-local,
 * puis se replie sur le pool global si nécessaire.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param size Taille du bloc de mémoire à allouer
 * @return void* Pointeur vers la mémoire allouée, NULL en cas d'échec
 */
void* valloc_block(MemoryAllocator* allocator, size_t size);

/**
 * @brief Recycle un bloc de mémoire pour une utilisation future
 * 
 * Au lieu de libérer la mémoire, marque le bloc comme recyclé
 * pour une réutilisation rapide.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param ptr Pointeur vers le bloc de mémoire à recycler
 */
void revalloc(MemoryAllocator* allocator, void* ptr);

/**
 * @brief Libère complètement un bloc de mémoire
 * 
 * Retourne la mémoire au système si elle ne peut pas être mise en cache.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @param ptr Pointeur vers le bloc de mémoire à libérer
 */
void free_valloc(MemoryAllocator* allocator, void* ptr);

/**
 * @brief Effectue le nettoyage des blocs recyclés
 * 
 * Libère les blocs de mémoire qui ont été recyclés mais non réutilisés.
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 */
void valloc_cleanup(MemoryAllocator* allocator);

/**
 * @brief Détruit l'allocateur et libère toute la mémoire
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 */
void valloc_destroy(MemoryAllocator* allocator);

/**
 * @brief Obtient le cache thread-local pour le thread actuel
 * 
 * @param allocator Pointeur vers la structure de l'allocateur
 * @return ThreadCache* Pointeur vers le cache thread-local
 */
ThreadCache* get_thread_cache(MemoryAllocator* allocator);

/**
 * @brief Alloue de la mémoire depuis le cache thread-local
 * 
 * @param cache Pointeur vers le cache thread-local
 * @param size Taille du bloc de mémoire à allouer
 * @return void* Pointeur vers la mémoire allouée, NULL en cas d'échec
 */
void* cache_allocate(ThreadCache* cache, size_t size);

/**
 * @brief Libère de la mémoire vers le cache thread-local
 * 
 * @param cache Pointeur vers le cache thread-local
 * @param ptr Pointeur vers le bloc de mémoire à libérer
 * @param size Taille du bloc de mémoire à libérer
 */
void cache_free(ThreadCache* cache, void* ptr, size_t size);

#endif // VALLOC_H

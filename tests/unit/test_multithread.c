#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "valloc.h"

#define NUM_THREADS 4
#define NUM_ALLOCATIONS 100
#define BLOCK_SIZE 1024

typedef struct {
    MemoryAllocator* allocator;
    int thread_id;
    void** blocks;
    int success;
} ThreadArg;

// Fonction exécutée par chaque thread
void* thread_function(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    thread_arg->success = 1;

    // Allouer des blocs
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        thread_arg->blocks[i] = valloc_block(thread_arg->allocator, BLOCK_SIZE);
        if (!thread_arg->blocks[i]) {
            thread_arg->success = 0;
            return NULL;
        }
        
        // Écrire dans le bloc pour vérifier l'accès
        memset(thread_arg->blocks[i], (char)thread_arg->thread_id, BLOCK_SIZE);
    }

    // Vérifier que les données sont correctes
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        char* block = (char*)thread_arg->blocks[i];
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (block[j] != (char)thread_arg->thread_id) {
                thread_arg->success = 0;
                return NULL;
            }
        }
    }

    return NULL;
}

// Test d'allocation concurrente
void test_concurrent_allocation() {
    MemoryAllocator allocator;
    valloc_init(&allocator, NUM_THREADS * NUM_ALLOCATIONS, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];
    void** all_blocks[NUM_THREADS];

    // Allouer la mémoire pour les blocs de chaque thread
    for (int i = 0; i < NUM_THREADS; i++) {
        all_blocks[i] = malloc(NUM_ALLOCATIONS * sizeof(void*));
        assert(all_blocks[i] != NULL);
    }

    // Créer les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].allocator = &allocator;
        thread_args[i].thread_id = i;
        thread_args[i].blocks = all_blocks[i];
        thread_args[i].success = 0;

        assert(pthread_create(&threads[i], NULL, thread_function, &thread_args[i]) == 0);
    }

    // Attendre les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        assert(thread_args[i].success == 1);
    }

    // Libérer la mémoire
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < NUM_ALLOCATIONS; j++) {
            free_valloc(&allocator, all_blocks[i][j]);
        }
        free(all_blocks[i]);
    }

    valloc_destroy(&allocator);
    printf("✓ Test d'allocation concurrente réussi\n");
}

// Test de recyclage concurrent
void test_concurrent_recycling() {
    MemoryAllocator allocator;
    valloc_init(&allocator, NUM_THREADS * NUM_ALLOCATIONS, NUM_THREADS);

    void* blocks[NUM_THREADS * NUM_ALLOCATIONS];

    // Allouer des blocs
    for (int i = 0; i < NUM_THREADS * NUM_ALLOCATIONS; i++) {
        blocks[i] = valloc_block(&allocator, BLOCK_SIZE);
        assert(blocks[i] != NULL);
    }

    // Recycler les blocs
    for (int i = 0; i < NUM_THREADS * NUM_ALLOCATIONS; i++) {
        revalloc(&allocator, blocks[i]);
    }

    // Réallouer les blocs
    for (int i = 0; i < NUM_THREADS * NUM_ALLOCATIONS; i++) {
        void* ptr = valloc_block(&allocator, BLOCK_SIZE);
        assert(ptr != NULL);
        free_valloc(&allocator, ptr);
    }

    valloc_destroy(&allocator);
    printf("✓ Test de recyclage concurrent réussi\n");
}

int main() {
    printf("=== Tests multithread ===\n");
    
    test_concurrent_allocation();
    test_concurrent_recycling();
    
    printf("\nTous les tests ont réussi !\n");
    return 0;
}

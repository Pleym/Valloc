#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "../../src/valloc.h"

#define NUM_THREADS 4
#define NUM_ALLOCATIONS 1000
#define BLOCK_SIZE 1024
#define INITIAL_BLOCKS 10000

MemoryAllocator allocator;

void* test_thread(void* arg) {
    int thread_num = *(int*)arg;
    void* ptrs[NUM_ALLOCATIONS];
    
    // Test d'allocations multiples
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        ptrs[i] = valloc_block(&allocator, BLOCK_SIZE);
        assert(ptrs[i] != NULL);
    }

    // Test de libération et réutilisation
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        revalloc(&allocator, ptrs[i]);
    }

    // Test de réallocation rapide (devrait utiliser le cache)
    void* quick_ptr = valloc_block(&allocator, BLOCK_SIZE);
    assert(quick_ptr != NULL);
    revalloc(&allocator, quick_ptr);

    printf("Thread %d completed successfully\n", thread_num);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_nums[NUM_THREADS];

    // Initialisation de l'allocateur
    if (valloc_init(&allocator, INITIAL_BLOCKS, NUM_THREADS) != 0) {
        printf("Failed to initialize allocator\n");
        return 1;
    }

    // Création des threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_nums[i] = i;
        if (pthread_create(&threads[i], NULL, test_thread, &thread_nums[i]) != 0) {
            printf("Failed to create thread %d\n", i);
            return 1;
        }
    }

    // Attente des threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Nettoyage
    valloc_cleanup(&allocator);
    printf("All tests passed successfully!\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "valloc.h"

#define NUM_THREADS 8
#define NUM_ITERATIONS 1000
#define MAX_BLOCK_SIZE 1048576  // 1MB

typedef struct {
    MemoryAllocator* allocator;
    int thread_id;
    int success;
} ThreadArg;

// Fonction pour obtenir une taille aléatoire
size_t get_random_size() {
    // Tailles possibles : 16B, 256B, 4KB, 16KB, 64KB, 256KB, 1MB
    const size_t sizes[] = {16, 256, 4096, 16384, 65536, 262144, 1048576};
    return sizes[rand() % 7];
}

// Fonction exécutée par chaque thread
void* stress_function(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    thread_arg->success = 1;

    // Tableau pour stocker les pointeurs alloués
    void* allocated[NUM_ITERATIONS];
    size_t sizes[NUM_ITERATIONS];
    int allocated_count = 0;

    for (int i = 0; i < NUM_ITERATIONS * 2; i++) {
        if (rand() % 2 == 0 && allocated_count < NUM_ITERATIONS) {
            // Allocation
            size_t size = get_random_size();
            void* ptr = valloc_block(thread_arg->allocator, size);
            
            if (!ptr) {
                thread_arg->success = 0;
                break;
            }

            // Écrire dans le bloc pour vérifier l'accès
            memset(ptr, (char)thread_arg->thread_id, size);
            
            allocated[allocated_count] = ptr;
            sizes[allocated_count] = size;
            allocated_count++;
        } 
        else if (allocated_count > 0) {
            // Libération ou recyclage
            int index = rand() % allocated_count;
            
            if (rand() % 2 == 0) {
                free_valloc(thread_arg->allocator, allocated[index]);
            } else {
                revalloc(thread_arg->allocator, allocated[index]);
            }

            // Déplacer le dernier élément à la position libérée
            allocated[index] = allocated[allocated_count - 1];
            sizes[index] = sizes[allocated_count - 1];
            allocated_count--;
        }
    }

    // Libérer les blocs restants
    for (int i = 0; i < allocated_count; i++) {
        free_valloc(thread_arg->allocator, allocated[i]);
    }

    return NULL;
}

// Test de stress avec allocation/libération aléatoire
void test_stress() {
    MemoryAllocator allocator;
    valloc_init(&allocator, NUM_THREADS * NUM_ITERATIONS, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];

    // Créer les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].allocator = &allocator;
        thread_args[i].thread_id = i;
        thread_args[i].success = 0;

        assert(pthread_create(&threads[i], NULL, stress_function, &thread_args[i]) == 0);
    }

    // Attendre les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        assert(thread_args[i].success == 1);
    }

    valloc_destroy(&allocator);
    printf("✓ Test de stress réussi\n");
}

int main() {
    printf("=== Test de stress ===\n");
    
    // Initialiser le générateur de nombres aléatoires
    srand(time(NULL));
    
    test_stress();
    
    printf("\nTous les tests ont réussi !\n");
    return 0;
}

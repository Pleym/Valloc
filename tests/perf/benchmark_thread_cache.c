#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "../../src/valloc.h"

#define NUM_THREADS 4
#define NUM_ITERATIONS 10000
#define BLOCK_SIZE 1024
#define INITIAL_BLOCKS 20000
#define CSV_FILE "benchmark_thread_cache.csv"

typedef struct {
    int thread_id;
    double execution_time;
    int use_cache;
} ThreadData;

MemoryAllocator allocator;
pthread_mutex_t csv_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* csv_file = NULL;

// Fonction pour mesurer le temps en nanosecondes
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void* benchmark_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    void* ptrs[NUM_ITERATIONS];
    
    double start_time = get_time();

    // Test d'allocations et libérations
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptrs[i] = valloc_block(&allocator, BLOCK_SIZE);
        if (i > 0) {
            revalloc(&allocator, ptrs[i-1]);
        }
    }
    revalloc(&allocator, ptrs[NUM_ITERATIONS-1]);

    double end_time = get_time();
    data->execution_time = (end_time - start_time) / 1e9; // Conversion en secondes

    // Écriture des résultats dans le fichier CSV
    pthread_mutex_lock(&csv_mutex);
    fprintf(csv_file, "%d,%d,%.9f\n", data->thread_id, data->use_cache, data->execution_time);
    pthread_mutex_unlock(&csv_mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    // Ouverture du fichier CSV
    csv_file = fopen(CSV_FILE, "w");
    if (!csv_file) {
        printf("Failed to open CSV file\n");
        return 1;
    }
    fprintf(csv_file, "thread_id,use_cache,execution_time\n");

    // Test avec cache
    if (valloc_init(&allocator, INITIAL_BLOCKS, NUM_THREADS) != 0) {
        printf("Failed to initialize allocator\n");
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].use_cache = 1;
        if (pthread_create(&threads[i], NULL, benchmark_thread, &thread_data[i]) != 0) {
            printf("Failed to create thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    valloc_cleanup(&allocator);

    // Test sans cache (simulation en désactivant le cache)
    if (valloc_init(&allocator, INITIAL_BLOCKS, 0) != 0) {  // 0 threads = pas de cache
        printf("Failed to initialize allocator\n");
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].use_cache = 0;
        if (pthread_create(&threads[i], NULL, benchmark_thread, &thread_data[i]) != 0) {
            printf("Failed to create thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    valloc_cleanup(&allocator);
    fclose(csv_file);

    printf("Benchmark completed. Results written to %s\n", CSV_FILE);
    return 0;
}

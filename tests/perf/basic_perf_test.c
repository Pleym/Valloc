#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "valloc.h"
#include "test_utils.h"

// Configuration des tests
#define NUM_ITERATIONS 1000
#define NUM_TESTS 5
#define MIN_SIZE 16       // 16 bytes
#define MAX_SIZE 1048576  // 1MB
#define INITIAL_BLOCKS 10000

int main() {
    // Initialisation de l'allocateur
    MemoryAllocator allocator;
    if (valloc_init(&allocator, INITIAL_BLOCKS, 1) != 0) {
        fprintf(stderr, "Erreur d'initialisation de l'allocateur\n");
        return 1;
    }

    // Ouvrir le fichier CSV
    FILE* csv_file = init_csv_file("basic_performance_results.csv");
    if (!csv_file) {
        valloc_destroy(&allocator);
        return 1;
    }

    // Tailles à tester (puissances de 2)
    size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 
                      8192, 16384, 32768, 65536, 131072, 262144, 
                      524288, 1048576};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    // Effectuer les tests pour chaque taille
    for (int i = 0; i < num_sizes; i++) {
        double start_time, end_time;
        void* ptr;

        // Test valloc allocation
        start_time = get_time();
        ptr = valloc_block(&allocator, sizes[i]);
        end_time = get_time();
        if (ptr == NULL) {
            fprintf(stderr, "Erreur d'allocation avec valloc pour la taille %zu\n", sizes[i]);
            continue;
        }
        TestResult valloc_alloc = {
            .allocator = "valloc",
            .size = sizes[i],
            .operation = "allocation",
            .time = end_time - start_time
        };
        write_result_to_csv(csv_file, valloc_alloc);

        // Test valloc free
        start_time = get_time();
        revalloc(&allocator, ptr);
        end_time = get_time();
        TestResult valloc_free = {
            .allocator = "valloc",
            .size = sizes[i],
            .operation = "free",
            .time = end_time - start_time
        };
        write_result_to_csv(csv_file, valloc_free);

        // Test malloc allocation
        start_time = get_time();
        ptr = malloc(sizes[i]);
        end_time = get_time();
        if (ptr == NULL) {
            fprintf(stderr, "Erreur d'allocation avec malloc pour la taille %zu\n", sizes[i]);
            continue;
        }
        TestResult malloc_alloc = {
            .allocator = "malloc",
            .size = sizes[i],
            .operation = "allocation",
            .time = end_time - start_time
        };
        write_result_to_csv(csv_file, malloc_alloc);

        // Test malloc free
        start_time = get_time();
        free(ptr);
        end_time = get_time();
        TestResult malloc_free = {
            .allocator = "malloc",
            .size = sizes[i],
            .operation = "free",
            .time = end_time - start_time
        };
        write_result_to_csv(csv_file, malloc_free);
    }

    // Nettoyage
    close_csv_file(csv_file);
    valloc_destroy(&allocator);

    printf("Tests terminés. Résultats sauvegardés dans basic_performance_results.csv\n");
    return 0;
}

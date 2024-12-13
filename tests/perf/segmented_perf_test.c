#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "valloc.h"
#include "../test_utils.h"

#define NUM_ITERATIONS 1000
#define MAX_ALLOCATION_SIZE 1048576  // 1MB
#define NUM_TESTS 5  // Nombre de tests à effectuer pour la moyenne
#define MAX_TEXT_SIZE 4096
#define MAX_RESULTS 1000

TestResult results[MAX_RESULTS];  // Tableau pour stocker tous les résultats
int result_count = 0;

// Fonction pour sauvegarder un résultat
void save_result(const char* allocator, size_t size, double time, const char* operation) {
    if (result_count >= MAX_RESULTS) return;
    results[result_count].allocator = allocator;
    results[result_count].size = size;
    results[result_count].time = time;
    results[result_count].operation = operation;
    result_count++;
}

// Test d'allocation et désallocation en boucle
void test_allocation_loop(size_t size, const char* allocator_name) {
    MemoryAllocator allocator;
    valloc_init(&allocator, NUM_ITERATIONS, 4);

    void* ptrs[NUM_ITERATIONS];
    double total_time = 0;

    for (int test = 0; test < NUM_TESTS; test++) {
        double start_time = get_time();
        
        // Test d'allocation
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            ptrs[i] = valloc_block(&allocator, size);
            if (!ptrs[i]) {
                fprintf(stderr, "Échec de l'allocation\n");
                exit(1);
            }
        }

        // Test de désallocation
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            free_valloc(&allocator, ptrs[i]);
        }

        double end_time = get_time();
        total_time += (end_time - start_time);
    }

    double avg_time = total_time / NUM_TESTS;
    save_result(allocator_name, size, avg_time, "allocation_loop");

    valloc_destroy(&allocator);
}

// Test de lecture/écriture de texte
void test_text_operations(const char* allocator_name) {
    MemoryAllocator allocator;
    valloc_init(&allocator, NUM_ITERATIONS, 4);

    char test_text[MAX_TEXT_SIZE];
    memset(test_text, 'A', MAX_TEXT_SIZE - 1);
    test_text[MAX_TEXT_SIZE - 1] = '\0';

    double total_time = 0;

    for (int test = 0; test < NUM_TESTS; test++) {
        double start_time = get_time();
        
        // Allocation et écriture
        void* ptr = valloc_block(&allocator, MAX_TEXT_SIZE);
        if (!ptr) {
            fprintf(stderr, "Échec de l'allocation\n");
            exit(1);
        }
        memcpy(ptr, test_text, MAX_TEXT_SIZE);

        // Lecture et vérification
        if (memcmp(ptr, test_text, MAX_TEXT_SIZE) != 0) {
            fprintf(stderr, "Erreur de vérification des données\n");
            exit(1);
        }

        free_valloc(&allocator, ptr);

        double end_time = get_time();
        total_time += (end_time - start_time);
    }

    double avg_time = total_time / NUM_TESTS;
    save_result(allocator_name, MAX_TEXT_SIZE, avg_time, "text_operations");

    valloc_destroy(&allocator);
}

// Fonction pour sauvegarder les résultats dans un fichier CSV
void save_results_to_csv() {
    FILE* file = fopen("segmented_test_results.csv", "w");
    if (!file) {
        fprintf(stderr, "Impossible d'ouvrir le fichier de résultats\n");
        return;
    }

    fprintf(file, "Allocator,Size,Operation,Time\n");
    for (int i = 0; i < result_count; i++) {
        write_result_to_csv(file, results[i]);
    }

    fclose(file);
}

int main() {
    const char* allocator_name = "valloc";
    
    // Test avec différentes tailles d'allocation
    for (size_t size = 16; size <= MAX_ALLOCATION_SIZE; size *= 2) {
        test_allocation_loop(size, allocator_name);
    }

    // Test de lecture/écriture de texte
    test_text_operations(allocator_name);

    // Sauvegarde des résultats
    save_results_to_csv();

    printf("Tests terminés. Les résultats ont été sauvegardés dans segmented_test_results.csv\n");
    return 0;
}

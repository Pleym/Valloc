#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "valloc.h"

// Structure pour stocker les résultats des tests
typedef struct {
    const char* allocator;
    size_t size;
    const char* operation;
    double time;
} TestResult;

// Fonction pour obtenir le temps en secondes
static double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Fonction pour écrire les résultats dans un fichier CSV
static void write_result_to_csv(FILE* file, TestResult result) {
    fprintf(file, "%s,%zu,%s,%f\n", 
            result.allocator,
            result.size,
            result.operation,
            result.time);
}

// Fonction pour initialiser un fichier CSV
static FILE* init_csv_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }
    fprintf(file, "allocator,size,operation,time\n");
    return file;
}

// Fonction pour fermer le fichier CSV
static void close_csv_file(FILE* file) {
    if (file) {
        fclose(file);
    }
}

#endif // TEST_UTILS_H

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>


// Lit le contenu d'un fichier et retourne un pointeur vers le contenu
// size contiendra la taille du fichier lu

static char* read_file(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    // Obtenir la taille du fichier
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    // Allouer la mémoire pour le contenu
    char* content = (char*)malloc(*size + 1);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }

    // Lire le contenu
    size_t read_size = fread(content, 1, *size, file);
    content[read_size] = '\0';

    fclose(file);
    return content;
}

#endif // TEST_UTILS_H

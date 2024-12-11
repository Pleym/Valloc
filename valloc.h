#ifndef VALLOC_H
#define VALLOC_H

#define MIN_BLOCK_SIZE 16    // Plus petite taille de bloc (2^4)
#define SMALL_BLOCK_SIZE 128
#define NUM_SIZE_CLASSES 9   // Nombre de classes de tailles (de 2^4 à 2^12)
#define MAX_BLOCK_SIZE 4096  // Plus grande taille de bloc (2^12)



#include <stddef.h>


typedef struct block_header {
    size_t size;           
    struct block_header* next;  
    struct block_header* prev;  
    int is_free;          
} block_header_t;


typedef struct {
    block_header_t* free_lists[NUM_SIZE_CLASSES];  
    void* heap_start;                              
} memory_manager_t;


void* valloc(size_t size);
void vafree(void* ptr);
void init_vallocator(void);
void cleanup_vallocator(void);

#endif // VALLOC_H

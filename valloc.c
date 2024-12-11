#include "valloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

static memory_manager_t memory_manager = {0};


static int get_size_class(size_t size) {
    if (size < MIN_BLOCK_SIZE) return 0;
    int class = (int)(log2(size - 1)) - 3;  
    if (class >= NUM_SIZE_CLASSES) return NUM_SIZE_CLASSES - 1;
    return class;
}


static size_t get_block_size(int size_class) {
    return 1 << (size_class + 4);  
}

void init_vallocator(void) {
    size_t initial_size = MAX_BLOCK_SIZE * 4;  
    void* heap = mmap(NULL, initial_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);
    
    if (heap == MAP_FAILED) {
        perror("Failed to initialize allocator");
        return;
    }

    memory_manager.heap_start = heap;
    
    // Initialiser le premier bloc
    block_header_t* initial_block = (block_header_t*)heap;
    initial_block->size = initial_size - sizeof(block_header_t);
    initial_block->is_free = 1;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    
    // Placer dans la plus grande classe de taille
    memory_manager.free_lists[NUM_SIZE_CLASSES - 1] = initial_block;
}

static void split_block(block_header_t* block, int target_size_class) {
    size_t current_size = block->size;
    size_t target_size = get_block_size(target_size_class);
    
    while (current_size >= target_size * 2) {
        size_t new_size = current_size / 2;
        block_header_t* new_block = (block_header_t*)((char*)block + new_size + sizeof(block_header_t));
        
        new_block->size = new_size - sizeof(block_header_t);
        new_block->is_free = 1;
        new_block->next = NULL;
        new_block->prev = NULL;
        
        block->size = new_size - sizeof(block_header_t);
        
        // Ajouter le nouveau bloc à la liste appropriée
        int new_class = get_size_class(new_size);
        new_block->next = memory_manager.free_lists[new_class];
        if (memory_manager.free_lists[new_class]) {
            memory_manager.free_lists[new_class]->prev = new_block;
        }
        memory_manager.free_lists[new_class] = new_block;
        
        current_size = new_size;
    }
}

void* valloc(size_t size) {
    if (size == 0) return NULL;
    
    // Trouver la classe de taille appropriée
    int size_class = get_size_class(size + sizeof(block_header_t));
    
    // Chercher un bloc libre dans la classe appropriée
    for (int i = size_class; i < NUM_SIZE_CLASSES; i++) {
        if (memory_manager.free_lists[i] != NULL) {
            block_header_t* block = memory_manager.free_lists[i];
            
            // Retirer le bloc de la liste
            memory_manager.free_lists[i] = block->next;
            if (block->next) {
                block->next->prev = NULL;
            }
            
            // Diviser le bloc si nécessaire
            if (i > size_class) {
                split_block(block, size_class);
            }
            
            block->is_free = 0;
            block->next = NULL;
            block->prev = NULL;
            
            return (void*)((char*)block + sizeof(block_header_t));
        }
    }
    
    // Si aucun bloc n'est disponible, allouer un nouveau bloc
    size_t alloc_size = get_block_size(size_class);
    void* ptr = mmap(NULL, alloc_size + sizeof(block_header_t),
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
    
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    
    block_header_t* block = (block_header_t*)ptr;
    block->size = alloc_size;
    block->is_free = 0;
    block->next = NULL;
    block->prev = NULL;
    
    return (void*)((char*)block + sizeof(block_header_t));
}

void vafree(void* ptr) {
    if (!ptr) return;
    
    block_header_t* block = (block_header_t*)((char*)ptr - sizeof(block_header_t));
    block->is_free = 1;

    int size_class = get_size_class(block->size);
    block->next = memory_manager.free_lists[size_class];
    block->prev = NULL;
    
    if (memory_manager.free_lists[size_class]) {
        memory_manager.free_lists[size_class]->prev = block;
    }
    memory_manager.free_lists[size_class] = block;
}

void cleanup_vallocator(void) {
    if (memory_manager.heap_start) {
        munmap(memory_manager.heap_start, MAX_BLOCK_SIZE * 4);
        memory_manager.heap_start = NULL;
        for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
            memory_manager.free_lists[i] = NULL;
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "valloc.h"

// Test d'initialisation et de destruction
void test_init_destroy() {
    MemoryAllocator allocator;
    assert(valloc_init(&allocator, 100, 4) == 0);
    valloc_destroy(&allocator);
    printf("✓ Test d'initialisation et destruction réussi\n");
}

// Test d'allocation et libération simple
void test_alloc_free() {
    MemoryAllocator allocator;
    valloc_init(&allocator, 100, 4);

    // Test avec différentes tailles
    size_t sizes[] = {16, 256, 4096, 16384};
    for (int i = 0; i < 4; i++) {
        void* ptr = valloc_block(&allocator, sizes[i]);
        assert(ptr != NULL);
        free_valloc(&allocator, ptr);
    }

    valloc_destroy(&allocator);
    printf("✓ Test d'allocation et libération réussi\n");
}

// Test de recyclage des blocs
void test_block_recycling() {
    MemoryAllocator allocator;
    valloc_init(&allocator, 100, 4);

    // Allouer et recycler plusieurs blocs
    void* ptrs[10];
    size_t size = 1024;

    // Première allocation
    for (int i = 0; i < 10; i++) {
        ptrs[i] = valloc_block(&allocator, size);
        assert(ptrs[i] != NULL);
    }

    // Recycler les blocs
    for (int i = 0; i < 10; i++) {
        revalloc(&allocator, ptrs[i]);
    }

    // Réallouer avec la même taille
    void* new_ptrs[10];
    for (int i = 0; i < 10; i++) {
        new_ptrs[i] = valloc_block(&allocator, size);
        assert(new_ptrs[i] != NULL);
    }

    // Nettoyer
    for (int i = 0; i < 10; i++) {
        free_valloc(&allocator, new_ptrs[i]);
    }

    valloc_destroy(&allocator);
    printf("✓ Test de recyclage des blocs réussi\n");
}

int main() {
    printf("=== Tests des opérations de base ===\n");
    
    test_init_destroy();
    test_alloc_free();
    test_block_recycling();
    
    printf("\nTous les tests ont réussi !\n");
    return 0;
}

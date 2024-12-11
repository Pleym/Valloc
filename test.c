#include "valloc.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("Test de l'allocateur mémoire personnalisé\n");

    // Test 1: Allocation simple
    printf("\nTest 1: Allocation simple\n");
    char* str = (char*)my_malloc(16);
    if (str == NULL) {
        printf("❌ Échec de l'allocation\n");
        return 1;
    }
    strcpy(str, "Valloc");
    printf("✅ Vallocation réussie: %s\n", str);
    my_free(str);

    // Test 2: Allocation de taille nulle
    printf("\nTest 2: Allocation de taille nulle\n");
    void* null_ptr = my_malloc(0);
    if (null_ptr == NULL) {
        printf("✅ Vallocation de taille nulle correctement gérée\n");
    } else {
        printf("❌ L'allocation de taille nulle devrait retourner NULL\n");
        my_free(null_ptr);
    }

    // Test 3: Allocation et libération multiple
    printf("\nTest 3: Allocations multiples\n");
    int* numbers[5];
    for (int i = 0; i < 5; i++) {
        numbers[i] = (int*)my_malloc(sizeof(int));
        if (numbers[i] == NULL) {
            printf("❌ Échec de l'allocation %d\n", i);
            return 1;
        }
        *numbers[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        printf("Nombre %d: %d\n", i, *numbers[i]);
        my_free(numbers[i]);
    }
    printf("✅ Vallocations et libérations multiples réussies\n");

    printf("\nTous les tests sont terminés\n");
    return 0;
}

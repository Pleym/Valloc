# Valloc - Allocateur de Mémoire Multi-thread avec Cache

## Description
Valloc est un allocateur de mémoire multi-thread optimisé avec un système de cache local par thread. Il offre des performances améliorées grâce à son mécanisme de recyclage des blocs de mémoire et sa gestion efficace de la contention entre threads.

## Caractéristiques
- Cache local par thread pour réduire la contention
- Recyclage des blocs de mémoire pour minimiser les appels système
- Support multi-thread avec synchronisation optimisée
- Gestion efficace des grands blocs de mémoire
- Outils de benchmarking et tests de performance
- Comparaison avec les allocateurs standards (`malloc`, `free`)

## Prérequis
- GCC compiler
- Make build system
- Python >= 3.0 (pour les benchmarks)
- Python packages: pandas, matplotlib et seaborn (pour la visualisation)

## Installation
```bash
git clone [repository-url]
cd valloc
#Exécuter les scripts d'installation
make all
#Exécuter les tests unitaires
make test
#Exécuter les tests de performance
make perf
```

## Utilisation
```c
#include "valloc.h"

// Initialisation de l'allocateur avec 1000 blocs initiaux et support pour 4 threads
MemoryAllocator allocator;
if (valloc_init(&allocator, 1000, 4) != 0) {
    // Gestion de l'erreur
    return -1;
}

// Allocation de mémoire
void* ptr = valloc_block(&allocator, 1024);  // Alloue 1024 bytes
if (ptr == NULL) {
    // Gestion de l'erreur d'allocation
}

// Utilisation de la mémoire...

// Option 1: Recyclage de la mémoire (recommandé pour les allocations fréquentes)
revalloc(&allocator, ptr);

// Option 2: Libération complète de la mémoire
free_valloc(&allocator, ptr);

// Nettoyage périodique des blocs recyclés (optionnel)
valloc_cleanup(&allocator);

// Destruction de l'allocateur en fin de programme
valloc_destroy(&allocator);
```

## Tests et Benchmarks
Le projet inclut plusieurs types de tests :

### Tests Unitaires
```bash
make test
./tests/unit/test_thread_cache
```

### Tests de Performance
```bash
# Test de performance basique
./tests/perf/basic_perf_test

# Test du cache thread-local
./tests/perf/benchmark_thread_cache

# Génération des graphiques
python3 benchmark/plot_results.py
python3 benchmark/plot_thread_size.py
```

## Documentation
Les sources utilisé pour réaliser ce projet :
- [Memory Allocation Strategies](https://www.gingerbill.org/series/memory-allocation-strategies/)
- [Gestion de la Mémoire](https://www.univ-orleans.fr/lifo/membres/Mirian.Halfeld/Cours/SEBlois/SE2007-GestionMemo.pdf)
- [TCMalloc Design](https://google.github.io/tcmalloc/design.html)

## Rapport
Pour une analyse des performances et de l'implémentation [rapport complet](Rapport_valoc.pdf).

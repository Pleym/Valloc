# Allocateur Mémoire Personnalisé

Ce projet implémente un allocateur mémoire personnalisé en C utilisant les appels système `mmap` et `munmap`.

## Fonctionnalités

- `my_malloc`: Alloue un bloc de mémoire de taille spécifiée
- `my_free`: Libère un bloc de mémoire précédemment alloué

## Compilation et Exécution

Pour compiler le projet :
```bash
make
```

Pour exécuter les tests :
```bash
./test
```

Pour nettoyer les fichiers compilés :
```bash
make clean
```

## Choix d'Implémentation

### Version de Base
- Utilisation directe de `mmap` pour chaque allocation
- Utilisation de `munmap` pour chaque libération
- Structure d'en-tête pour stocker les métadonnées des blocs
- Alignement sur la taille de page pour optimiser l'utilisation mémoire

### Points d'Optimisation Possibles
1. Implémentation d'un cache de blocs libérés
2. Coalescence des blocs adjacents
3. Segmentation en classes de tailles
4. Support multi-thread
5. Optimisation des métadonnées

## Structure du Code

- `valloc.h`: Déclarations des fonctions et structures
- `valloc.c`: Implémentation de l'allocateur
- `test.c`: Programme de test
- `Makefile`: Script de compilation

CC = gcc
CFLAGS = -Wall -Wextra -I./src -I./tests
LDFLAGS = 

# Répertoires
SRC_DIR = src
TEST_DIR = tests
PERF_DIR = $(TEST_DIR)/perf
UNIT_DIR = $(TEST_DIR)/unit

# Fichiers sources
SRC = $(SRC_DIR)/valloc.c
TEST_SOURCES = $(wildcard $(UNIT_DIR)/*.c)
PERF_SOURCES = $(wildcard $(PERF_DIR)/*.c)

# Fichiers objets
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
PERF_OBJECTS = $(PERF_SOURCES:.c=.o)

# Exécutables
TEST_EXECUTABLES = $(TEST_SOURCES:.c=)
PERF_EXECUTABLES = $(PERF_SOURCES:.c=)

# Cibles principales
.PHONY: all clean test perf

all: $(TEST_EXECUTABLES) $(PERF_EXECUTABLES)

# Règle pour les tests unitaires
$(UNIT_DIR)/%: $(UNIT_DIR)/%.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règle pour les tests de performance
$(PERF_DIR)/%: $(PERF_DIR)/%.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Exécution des tests unitaires
test: $(TEST_EXECUTABLES)
	@echo "Exécution des tests unitaires..."
	@for test in $(TEST_EXECUTABLES); do \
		echo "Exécution de $$test"; \
		$$test; \
	done

# Exécution des tests de performance
perf: $(PERF_EXECUTABLES)
	@echo "Exécution des tests de performance..."
	@for test in $(PERF_EXECUTABLES); do \
		echo "Exécution de $$test"; \
		$$test; \
	done

# Nettoyage
clean:
	rm -f $(TEST_EXECUTABLES) $(PERF_EXECUTABLES)
	rm -f $(TEST_DIR)/*.o $(SRC_DIR)/*.o
	rm -f *.csv

# Valloc - A High-Performance Memory Allocator
![Valloc Logo](src/logo.png)

## Overview
Valloc is a custom memory allocator implemented in C, designed to provide efficient memory management through `mmap` and `munmap` system calls. It features thread-safe operations and competitive performance compared to standard allocators.

## Features
- Custom memory allocation and deallocation
- Thread-safe operations with mutex protection
- Support for various block sizes
- Performance benchmarking tools
- Comparative analysis against standard allocators (`malloc`, `calloc`)

## Performance
![Performance Comparison](performance_comparison.png)

Our latest benchmarks show:
- Optimal performance for large block sizes (>256KB)
- Competitive with standard allocators for memory-intensive operations
- Thread-safe operations with minimal overhead

## Building from Source

### Prerequisites
- GCC compiler
- Make build system
- Python 3.x (for benchmarking)
- Python packages: pandas, matplotlib (for visualization)

### Installation
```bash
# Clone the repository
git clone https://github.com/yourusername/valloc.git
cd valloc

# Build the project
make all

# Run tests
make test
```

## Usage
```c
#include "valloc.h"

// Initialize the allocator
valloc_init();

// Allocate memory
void* ptr = valloc_block(1024);  // Allocate 1024 bytes

// Use the memory
// ...

// Free the memory
revalloc(ptr);

// Cleanup
valloc_bye();
```

## Benchmarking
The project includes comprehensive benchmarking tools:
```bash
# Run performance tests
./bin/multithread_comparison_test

# Generate visualization
python3 benchmark/plot_thread_size.py
```



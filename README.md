# Parallel Sudoku Solver

A high-performance parallel Sudoku solver implemented in C using POSIX threads.

## Overview

This project implements both serial and parallel algorithms for solving Sudoku puzzles, with a focus on performance optimization through:

- Thread-based parallelism
- Efficient work queue distribution
- Bit manipulation for fast validity checking
- Hybrid approach combining breadth-first search with backtracking

## Features

- Supports boards ranging from standard 9×9 up to 64×64 boards
- Configurable number of worker threads
- Adjustable recursion depth before switching to backtracking
- Batch processing of tasks to reduce synchronization overhead
- Performance metrics and thread utilization statistics

## Building

```bash
make
```

## Usage
```bash
./main <BASE> <GRID_FILENAME> <NUM_THREADS> <BASE_DEPTH> <MINIMUM_TASK_COUNT>
```

#### Arguments:

- `<BASE>` — Base size of one Sudoku box (e.g. 3 for 9×9)
- `<GRID_FILENAME>` — Path to a binary grid file (e.g. `grids/grid_9x9.dat`)
- `<NUM_THREADS>` — Number of worker threads
- `<BASE_DEPTH>` — Recursion depth before switching to backtracking
- `<MINIMUM_TASK_COUNT>` — Minimum number of tasks to keep in the work queue

### Example

```bash
./main 6 grids/grid_36x36.dat 8 74 1000
```

## Requirements

- POSIX-compliant OS (Linux, macOS)
- GCC or compatible compiler supporting:
    - C99 or newer
    - OpenMP (for timing only)
- Tested on:
    - x86_64 Linux
    - macOS (Apple Silicon)

© 2025 Carl Löfkvist  
Licensed under the MIT License.
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
./main <BASE> <N_THREADS> <RECURSIVE_DEPTH_LIMIT> <MIN_TASKS_IN_QUEUE> <TASK_BATCH_SIZE>
```

#include "init_sudoku.h"
#include <stdlib.h>
#include <stdatomic.h>

#define DEQUE_SIZE 1024  // Can be adjusted based on problem size


typedef struct {
    Sudoku* sudoku; // Pointer to own dopy
    int row, col; // Position to test
} Task;

Sudoku *deep_copy_sudoku(Sudoku *parent);
Task *create_task(Sudoku *parent, int row, int col);

typedef struct {
    Task *tasks[DEQUE_SIZE];       // Array of tasks
    atomic_int top;     // Index for work stealing
    atomic_int bottom;         // Local worker index
    int capacity;       // Max task capacity
} WorkDeque;

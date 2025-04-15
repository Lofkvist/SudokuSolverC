#include "parallel.h"
#include "display_functions.h"
#include "init_sudoku.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

Sudoku *deep_copy_sudoku(Sudoku *parent);

Sudoku* deep_copy_sudoku(Sudoku* parent) {
    Sudoku* child = malloc(sizeof(Sudoku));
    if (!child) {
        return NULL;  // Return NULL instead of exit
    }
    
    child->base = parent->base;
    child->len = parent->len;
    int len = child->len;
    
    // Allocate grid memory
    size_t grid_size = len * len * sizeof(Cell);
    child->grid = malloc(grid_size);
    if (!child->grid) {
        free(child);
        return NULL;  // Return NULL instead of exit
    }
    
    // Fast copy all cells at once
    memcpy(child->grid, parent->grid, grid_size);
    
    return child;
}
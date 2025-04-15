// sudoku.h
#ifndef SUDOKU_H
#define SUDOKU_H
#include <stdio.h>
#include <stdlib.h>

typedef struct Cell{
    int value;
} Cell;

typedef struct {
    int base;
    int len;
    Cell *grid;
    
    // Add bitmasks for fast validity checking
    unsigned long long *row_bits;  // Bit flags for each row
    unsigned long long *col_bits;  // Bit flags for each column
    unsigned long long *box_bits;  // Bit flags for each box
} Sudoku;

Sudoku *init_sudoku(int N);
void free_sudoku(Sudoku *sudoku);
#endif // SUDOKU_H
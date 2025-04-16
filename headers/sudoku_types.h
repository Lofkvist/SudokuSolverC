// sudoku_types.h
#ifndef SUDOKUTYPES_H
#define SUDOKUTYPES_H

#include <stdio.h>
#include <stdlib.h>

/**
 * Single cell in a Sudoku puzzle
 */
typedef struct Cell {
    int value;  // 0 indicates empty cell
} Cell;

/**
 * Coordinates of a cell with a found flag
 */
typedef struct coord {
    int r;      // Row index
    int c;      // Column index
    int found;  // Flag indicating if a cell was found
} coord_t;

/**
 * Complete Sudoku puzzle with all data structures
 */
typedef struct {
    int base;   // Base size (e.g., 3 for standard 9x9 puzzle)
    int len;    // Full length (base^2, e.g., 9 for standard puzzle)
    Cell *grid; // 1D array of cells for the entire grid

    // Bitmasks for fast validity checking
    unsigned long long *row_bits;  // Bit flags for each row
    unsigned long long *col_bits;  // Bit flags for each column
    unsigned long long *box_bits;  // Bit flags for each box
} Sudoku;

#endif // SUDOKUTYPES_H
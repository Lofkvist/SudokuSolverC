// sudoku_types.h
#ifndef SUDOKUTYPES_H
#define SUDOKUTYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**
 * Coordinates of a cell with a found flag
 */
typedef struct coord {
    uint8_t r;      // Row index
    uint8_t c;      // Column index
    uint8_t found;  // Flag indicating if a cell was found
} coord_t;

/**
 * Complete Sudoku puzzle with all data structures
 */
typedef struct {
    uint8_t base;
    uint8_t len; // base * base
    uint8_t *grid; // Sudoku grid

    // Bitmasks for fast validation
    unsigned long long *row_bits;
    unsigned long long *col_bits;
    unsigned long long *box_bits;
} Sudoku;

#endif // SUDOKUTYPES_H
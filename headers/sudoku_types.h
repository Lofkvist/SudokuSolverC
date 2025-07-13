// sudoku_types.h
#ifndef SUDOKUTYPES_H
#define SUDOKUTYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**
 * Represents the coordinates of a cell in the Sudoku grid,
 * along with a flag indicating whether an empty cell was found.
 */
typedef struct coord {
    uint8_t r;      // Row index (0-based)
    uint8_t c;      // Column index (0-based)
    uint8_t found;  // 1 if cell is found (empty), 0 otherwise
} coord_t;

/**
 * Represents the complete state of a Sudoku puzzle,
 * including:
 *   - base size (e.g. 3 for a 9x9 grid)
 *   - grid data
 *   - bitmasks for fast validity checks
 */
typedef struct {
    uint8_t base;         // Base size of the puzzle (e.g. 3 for 9x9)
    uint8_t len;          // Total size of one side (base * base)
    uint8_t *grid;        // Flattened grid of size len * len

    // Bitmasks for fast constraint checking
    unsigned long long *row_bits; // One bitmask per row
    unsigned long long *col_bits; // One bitmask per column
    unsigned long long *box_bits; // One bitmask per box
} Sudoku;

#endif // SUDOKUTYPES_H

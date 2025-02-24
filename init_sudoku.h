// sudoku.h
#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint64_t candidates; // Bit mask of possible numbers (9 bits used)
  uint8_t value;      // Current value (0 if unset)
} Cell;

typedef struct {
    uint8_t base; // Size
    Cell **grid; // The Sudoku grid
} Sudoku;

// Function to create a Sudoku board of size N×N
Sudoku* create_sudoku(uint8_t N);

// Function to free the allocated Sudoku board
void free_sudoku(Sudoku *sudoku);

#endif // SUDOKU_H

/*
// Pre-computed peer indices for fast access
row_peers
col_peers
box_peers

// Additional useful data for techniques
row_candidates // Candidates present in each row
col_candidates // Candidates present in each column
box_candidates // Candidates present in each box

// Statistics for optimization
candidates_count; // Count of candidates per cell
unsolved_count;         // Number of unsolved cells
*/
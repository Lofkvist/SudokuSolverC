// sudoku.h
#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdint.h>
#include <stdlib.h>

typedef struct Cell{
    uint_fast64_t candidates;
    int num_candidates;
    int value;
    struct Cell** row_peers;
    struct Cell** col_peers;
    struct Cell** box_peers;
} Cell;

typedef struct {
    int base;
    int len;
    Cell **grid;
    int unsolved_count;         // Number of unsolved cells
} Sudoku;

// Function to create a Sudoku board of size N×N
Sudoku *init_sudoku(int N);

// Function to free the allocated Sudoku board
void free_sudoku(Sudoku *sudoku);

void delete_from_peers(Cell* cell, int len);

void init_peer_candidates(Sudoku *sudoku);

void init_cell_peers(Sudoku *sudoku);

void populate_board(Sudoku *sudoku);

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
*/
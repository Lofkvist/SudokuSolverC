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
    Cell *grid;
} Sudoku;

/*
Initialize a board of size NxN

Initial clues are taken from the binary file boards/board_DxD.dat, where D = N*N
*/
Sudoku *init_sudoku(int N);

/*
Free all allocated memory
*/
void free_sudoku(Sudoku *sudoku);

/*
Remove the number placed in the cell from all its peers

If that bit was 1 beforehand => num_candidates is decremented
If that bit was 0 beforehand => num_candidates is unchanged
*/
void delete_from_peers(Cell* cell, int len);

#endif // SUDOKU_H
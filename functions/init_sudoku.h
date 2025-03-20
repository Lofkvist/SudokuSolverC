// sudoku.h
#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdint.h>
#include <stdlib.h>

typedef struct Cell{
    int value;
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

#endif // SUDOKU_H
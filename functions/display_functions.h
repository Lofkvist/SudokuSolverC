#ifndef SUDOKU_PRINT_H
#define SUDOKU_PRINT_H

#include "init_sudoku.h"
#include <stdint.h>

void print_binary(uint_fast64_t num, int len);
void print_sudoku(Sudoku *sudoku);
void print_candidates(Sudoku *sudoku);
void print_num_candidates(Sudoku *sudoku);
void python_print(Sudoku *sudoku);

#endif // SUDOKU_PRINT_H

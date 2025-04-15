#include "init_sudoku.h"
#include <stdio.h>

void print_sudoku(Sudoku *sudoku) {
    int len = sudoku->len;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", sudoku->grid[i * len + j].value);
        }
        printf("\n");
    }
}


void python_print(Sudoku *sudoku) {
    int len = sudoku->len;

    for (int i = 0; i < len; i++) {
        printf("[");
        for (int j = 0; j < len; j++) {
            printf("%d, ", sudoku->grid[i * len + j].value);
        }
        printf("],\n");
    }
}
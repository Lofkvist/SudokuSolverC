#include "init_sudoku.h"
#include <stdio.h>

void print_binary(uint_fast64_t num, int len) {
    // Print the bits from the highest bit (63) to the lowest bit (0)
    for (int i = len - 1; i >= 0; i--) {
        putchar((num & (1ULL << i)) ? '1' : '0');
    }
    printf(" ");
}

void print_sudoku(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", grid[i][j].value);
        }
        printf("\n");
    }
}

void print_candidates(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            print_binary(grid[i][j].candidates, len);
        }
        printf("\n");
    }
}

void print_num_candidates(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", grid[i][j].num_candidates);
        }
        printf("\n");
    }
}

void python_print(Sudoku *sudoku) {
    int len = sudoku->len;

    for (int i = 0; i < len; i++) {
        printf("[");
        for (int j = 0; j < len; j++) {
            printf("%d, ", sudoku->grid[i][j].value);
        }
        printf("],\n");
    }
}
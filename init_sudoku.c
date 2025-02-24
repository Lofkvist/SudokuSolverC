#include "init_sudoku.h"
#include <stdio.h>

Sudoku* create_sudoku(uint8_t N) {
    Sudoku *sudoku = malloc(sizeof(Sudoku));
    if (!sudoku) return NULL;

    sudoku->base = N;
    sudoku->grid = malloc(N * sizeof(Cell*));
    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    for (uint8_t i = 0; i < N; i++) {
        sudoku->grid[i] = calloc(N, sizeof(Cell)); // Initialize all to 0
        if (!sudoku->grid[i]) { // Allocation failed
            for (uint8_t j = 0; j < i; j++)
                free(sudoku->grid[j]);
            free(sudoku->grid);
            free(sudoku);
            return NULL;
        }
    }

    return sudoku;
}

void free_sudoku(Sudoku *sudoku) {
    for (uint8_t j = 0; j < i; j++)
                free(sudoku->grid[j]);
            free(sudoku->grid);
            free(sudoku);
};
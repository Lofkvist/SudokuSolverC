#include "init_sudoku.h"
#include <stdint.h>
#include <stdio.h>

static void populate_board(Sudoku *sudoku);

Sudoku *init_sudoku(uint_fast8_t N) {
    Sudoku *sudoku = malloc(sizeof(Sudoku));
    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    uint_fast64_t M = N * N;

    sudoku->grid = malloc(M * sizeof(Cell *));
    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    uint_fast8_t j, i;
    for (i = 0; i < M; i++) {
        sudoku->grid[i] = calloc(M, sizeof(Cell)); // Initialize all to 0
        if (!sudoku->grid[i]) {                    // Allocation failed
            for (j = 0; j < i; j++)
                free(sudoku->grid[j]);
            free(sudoku->grid);
            free(sudoku);
            return NULL;
        }
    }

    populate_board(sudoku);

    return sudoku;
}

void free_sudoku(Sudoku *sudoku) {
    uint_fast8_t M = sudoku->len;
    uint_fast8_t j;
    for (j = 0; j < M; j++)
        free(sudoku->grid[j]);
    free(sudoku->grid);
    free(sudoku);
};

static void populate_board(Sudoku *sudoku) {
    char filename[30];
    // Assuming placed in ./boards directory
    snprintf(filename, sizeof(filename), "boards/board_%dx%d.dat", sudoku->len,
             sudoku->len);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Read data from the binary file
    unsigned char data;

    // Read until the end of the file
    int i=0, j=0, k = 0;
    // Ignore first number, that is the base
    fread(&data, sizeof(data), 1, file);
    for (i = 0; i < sudoku->len; i++) {
        for (j = 0; j < sudoku->len; j++) {
            fread(&data, sizeof(data), 1, file);
            sudoku->grid[i][j].value = (uint_fast8_t) data;
        }
    }

    // Check for read errors
    if (ferror(file)) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    // Close the file
    fclose(file);
}
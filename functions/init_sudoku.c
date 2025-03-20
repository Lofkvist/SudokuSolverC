#include "init_sudoku.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void populate_board(Sudoku *sudoku);

Sudoku *init_sudoku(int N) {
    Sudoku *sudoku = malloc(sizeof(Sudoku));
    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    int len = N * N;

    sudoku->grid = malloc(len * len * sizeof(Cell));
    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    populate_board(sudoku);
    return sudoku;
}

// FREE PEER ARRAYS FOR EACH CELL TOO
void free_sudoku(Sudoku *sudoku) {
    // Row 0 points to all cell memory
    free(sudoku->grid);
    free(sudoku);
};

static void populate_board(Sudoku *sudoku) {
    char filename[40];
    int len = sudoku->len;
    Cell* grid = sudoku->grid;

    // Assuming placed in ./boards directory
    snprintf(filename, sizeof(filename), "boards/board_%dx%d.dat", len, len);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Skip first two bytes (already known)
    fseek(file, 2, SEEK_SET);

    // Read data from the binary file
    unsigned char *data = malloc(len * len);  // Buffer for faster reading
    if (!data) {
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(data, 1, len * len, file) != (size_t)(len * len)) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Vectorized loop
    for (int i = 0; i < len * len; i++) {
        grid[i].value = (int)data[i];
    }


    // Close the file
    free(data);
}
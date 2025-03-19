#include "init_sudoku.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void populate_board(Sudoku *sudoku);
static void init_cell_peers(Sudoku *sudoku);

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
    init_cell_peers(sudoku);
    return sudoku;
}

// FREE PEER ARRAYS FOR EACH CELL TOO
void free_sudoku(Sudoku *sudoku) {
    // Row peers in cell (0,0) points to all peer memory
    free(sudoku->grid[0].row_peers);
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

static void init_cell_peers(Sudoku *sudoku) {
    int len = sudoku->len;
    int base = sudoku->base;

    int r, c;
    int x, y;
    int peer_index;
    int total_peer_count = 3 * (len - 1) * len * len;
    Cell **all_peers = malloc(total_peer_count * sizeof(Cell *));
    if (all_peers == NULL) {
        printf("Memory allocation failed in init_cell_peers");
        exit(EXIT_FAILURE);
    }

    for (r = 0; r < len; r++) { // Rows
        for (c = 0; c < len; c++) { // Cols
            int peer_offset = 3 * (len - 1) * (r * len + c);

            // Peer memory slots
            sudoku->grid[r*len + c].row_peers = all_peers + peer_offset;
            sudoku->grid[r*len + c].col_peers = all_peers + (len - 1) + peer_offset;
            sudoku->grid[r*len + c].box_peers = all_peers + 2 * (len - 1) + peer_offset;

            // Row peers
            peer_index = 0;
            for (x = 0; x < len; x++) {
                if (x != c) { // Skip the cell itself
                    sudoku->grid[r*len + c].row_peers[peer_index++] = &sudoku->grid[r*len + x];
                }
            }

            // Col peers
            peer_index = 0;
            for (y = 0; y < len; y++) {
                if (y != r) { // Skip the cell itself
                    sudoku->grid[r*len + c].col_peers[peer_index++] = &sudoku->grid[y*len + c];
                }
            }

            // Box peers
            int box_start_row = (r / base) * base;
            int box_start_col = (c / base) * base;
            peer_index = 0;

            for (y = box_start_row; y < box_start_row + base; y++) {
                for (x = box_start_col; x < box_start_col + base; x++) {
                    if (y == r && x == c) {
                        continue; // Skip the cell itself
                    }
                    sudoku->grid[r*len + c].box_peers[peer_index++] = &sudoku->grid[y*len + x];
                }
            }
        }
    }
}
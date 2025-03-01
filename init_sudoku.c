#include "init_sudoku.h"
#include "cell_bit_operations.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void populate_board(Sudoku *sudoku);
static void init_cell_peers(Sudoku *sudoku);
static void init_peer_candidates(Sudoku *sudoku);
static void delete_from_peers(Cell *cell, int len);

// Cell and bit operations
int get_bit(uint_fast64_t candidates, int val);
void update_candidates(Cell* cell, int pos, int len);

Sudoku *init_sudoku(int N) {
    Sudoku *sudoku = malloc(sizeof(Sudoku));
    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    int M = N * N;

    sudoku->grid = malloc(M * sizeof(Cell *));
    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    int j, i;
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
    init_cell_peers(sudoku);
    init_peer_candidates(sudoku);

    return sudoku;
}

// FREE PEER ARRAYS FOR EACH CELL TOO
void free_sudoku(Sudoku *sudoku) {
    int M = sudoku->len;
    int j, i;
    for (i = 0; i < M; i++)
        for (j = 0; j < M; j++) {
            {
                free(sudoku->grid[i][j].row_peers);
            }
        }
    for (j = 0; j < M; j++)
        free(sudoku->grid[j]);
    free(sudoku->grid);
    free(sudoku);
};

static void populate_board(Sudoku *sudoku) {
    char filename[40];
    int len = sudoku->len;
    Cell **grid = sudoku->grid;

    // Assuming placed in ./boards directory
    snprintf(filename, sizeof(filename), "boards/board_%dx%d.dat", len, len);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Read data from the binary file
    unsigned char data;

    // Read until the end of the file
    int i = 0, j = 0, k = 0, unsolved_count = 0;

    // Ignore first number, that is the base
    if (!fread(&data, sizeof(data), 1, file)) {
        exit(EXIT_FAILURE);
    }

    // Set value and candidates field
    for (i = 0; i < len; i++) {
        Cell *row = grid[i];
        for (j = 0; j < len; j++) {
            if (!fread(&data, sizeof(data), 1, file)) {
                exit(EXIT_FAILURE);
            }
            row[j].value = (int)data;
            if (!data) { // No clue in this cell
                unsolved_count++;
                row[j].candidates = UINT_FAST64_MAX; // All options avaliable
                row[j].num_candidates = len;
            } else {                   // Clue given
                row[j].candidates = 0; // No other candidates for this cell
                row[j].num_candidates = 0;
            }
        }
    }
    sudoku->unsolved_count = unsolved_count;

    // Check for read errors
    if (ferror(file)) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    // Close the file
    fclose(file);
}

static void init_cell_peers(Sudoku *sudoku) {
    int len = sudoku->len;
    int base = sudoku->base;
    Cell **grid = sudoku->grid;

    int r, c;
    int x, y, z;
    int peer_index;
    for (r = 0; r < len; r++) { // Rows
        Cell *row = grid[r];
        for (c = 0; c < len; c++) { // Cols

            // Row peers
            Cell **all_peers = malloc(3 * (len - 1) * sizeof(Cell *));
            if (all_peers == NULL) {
                printf("Memory allocation failed in init_cell_peers");
                exit(EXIT_FAILURE);
            }

            row[c].row_peers = all_peers;
            row[c].col_peers = all_peers + (len - 1);
            row[c].box_peers = all_peers + 2 * (len - 1);

            // THE IF STATEMENT IN THESE LOOPS CAN BE REMOVED
            peer_index = 0; // Index for row_peers
            for (x = 0; x < c; x++)
                row[c].row_peers[peer_index++] = &row[x];

            for (x = c + 1; x < len; x++)
                row[c].row_peers[peer_index++] = &row[x];

            // Col peers
            peer_index = 0; // Index for row_peers
            for (y = 0; y < r; y++)
                row[c].col_peers[peer_index++] = &grid[y][c];
            for (y = r + 1; y < len; y++)
                row[c].col_peers[peer_index++] = &grid[y][c];

            // Box peers
            int br = r / base;
            int bc = c / base;
            peer_index = 0;

            for (x = br * base; x < (br + 1) * base; x++) {
                for (y = bc * base; y < (bc + 1) * base; y++) {
                    if (x == r && y == c) {
                        continue;
                    }
                    row[c].box_peers[peer_index++] = &grid[x][y];
                }
            }
        }
    }
}

/*
Sudoku *sudoku
*/

static void init_peer_candidates(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;

    int r, c;
    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (!grid[r][c].value)
                continue;                        // No hint, peers are unchanged
            delete_from_peers(&grid[r][c], len); // Remove cell value from peers
        }
    }
}

static void delete_from_peers(Cell* cell, int len) {
    int value = cell->value;
    uint_fast64_t check_mask = 1ULL << (value - 1);    // Mask to check if bit is set
    uint_fast64_t clear_mask = ~check_mask;            // Mask to clear the bit
    int j;

    // Update peers, update 
    for (j = 0; j < len-1; j++) {
        /*
        update_candidates(cell->box_peers[j], value, len);
        update_candidates(cell->row_peers[j], value, len);
        update_candidates(cell->col_peers[j], value, len);
        */

        cell->box_peers[j]->num_candidates -= get_bit(cell->box_peers[j]->candidates, value);
        cell->box_peers[j]->candidates &= clear_mask;
        
        // Row peers
        cell->row_peers[j]->num_candidates -= get_bit(cell->row_peers[j]->candidates, value);
        cell->row_peers[j]->candidates &= clear_mask;
        
        // Column peers
        cell->col_peers[j]->num_candidates -= get_bit(cell->col_peers[j]->candidates, value);
        cell->col_peers[j]->candidates &= clear_mask;

    }
}
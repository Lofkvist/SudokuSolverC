#include "init_sudoku.h"
#include <stdint.h>
#include <stdio.h>

static void populate_board(Sudoku *sudoku);
static void init_cell_peers(Sudoku *sudoku);

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
                free(sudoku->grid[i][j].col_peers);
            }
        }
    for (j = 0; j < M; j++)
        free(sudoku->grid[j]);
    free(sudoku->grid);
    free(sudoku);
};

static void populate_board(Sudoku *sudoku) {
    char filename[30];
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
    fread(&data, sizeof(data), 1, file);

    // Set value and candidates field
    for (i = 0; i < len; i++) {
        Cell *row = grid[i];
        for (j = 0; j < len; j++) {
            fread(&data, sizeof(data), 1, file);
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
    int base = sudoku->len;
    Cell **grid = sudoku->grid;

    int r, c;
    int x, y, z;
    int peer_index;
    for (r = 0; r < len; r++) { // Rows
        Cell *row = grid[r];
        for (c = 0; c < len; c++) { // Cols

            // Row peers
            //Cell **all_peers = malloc(3 * (len - 1) * sizeof(Cell *));

            row[c].row_peers = malloc((len - 1) * sizeof(Cell *));
            if (row[c].row_peers == NULL) {
                printf("Memory allocation failed in init_cell_peers");
                exit(EXIT_FAILURE);
            }
            // THE IF STATEMENT IN THESE LOOPS CAN BE REMOVED
            peer_index = 0; // Index for row_peers
            for (x = 0; x < len; x++) {
                if (x != c) { // Skip the current cell
                    row[c].row_peers[peer_index++] =
                        &row[x]; // Assign peer cells
                }
            }

            // Col peers
            row[c].col_peers = malloc((len - 1) * sizeof(Cell *));
            if (row[c].col_peers == NULL) {
                printf("Memory allocation failed in init_cell_peers");
                exit(EXIT_FAILURE);
            }
            peer_index = 0;             // Index for row_peers
            for (y = 0; y < len; y++) { // Loop rows
                if (y != r) {           // Skip the current cell
                    row[c].col_peers[peer_index++] = &grid[y][c];
                }
            }

            // Box peers
        }
    }
}
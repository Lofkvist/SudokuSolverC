/* --------------------------------------------------------
 * File:        sudoku_utils.c
 * Author:      Carl Löfkvist
 * Date:        2025-07-13
 * Description: Definition of utility functions used by both solvers
 * -------------------------------------------------------- */

#include "../headers/sudoku_types.h"
#include <string.h>
#include <stdint.h>

/* Forward declaration for static functions */
static void populate_grid(Sudoku* sudoku, char *grid_filename);


/**
 * Finds the first empty cell in the Sudoku grid.
 * Returns coord_t with .found = 1 if a cell is found,
 * otherwise .found = 0 if grid is completely filled.
 */
coord_t first_empty_cell(Sudoku* sudoku) {
    coord_t pos = {0, 0, 0};
    uint8_t len = sudoku->len;

    uint8_t r, c;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r * len + c] == 0) {
                pos.found = 1;
                pos.r = r;
                pos.c = c;
                return pos;
            }
        }
    }

    return pos;
}


/**
 * Checks if placing a number in a specific cell
 * is valid according to Sudoku rules.
 */
uint8_t is_valid_placement(Sudoku* sudoku, uint8_t r, uint8_t c, uint8_t num) {
    unsigned long long bit = 1ULL << (num - 1);
    uint8_t box = (r / sudoku->base) * sudoku->base + (c / sudoku->base);

    // If bit is set in any constraint, placement is invalid
    return !((sudoku->row_bits[r] | sudoku->col_bits[c] | sudoku->box_bits[box]) & bit);
}


/**
 * Creates a deep copy of a Sudoku structure
 * including grid data and bitmasks.
 */
Sudoku* deep_copy_sudoku(Sudoku* parent) {
    Sudoku* child = malloc(sizeof(Sudoku));
    if (!child) {
        return NULL;
    }

    child->base = parent->base;
    child->len = parent->len;
    uint8_t len = child->len;

    // Allocate grid memory
    int grid_size = len * len * sizeof(uint8_t);
    child->grid = malloc(grid_size);
    if (!child->grid) {
        free(child);
        return NULL;
    }

    memcpy(child->grid, parent->grid, grid_size);

    // Allocate bitmasks in one block
    child->row_bits = malloc(3 * len * sizeof(unsigned long long));
    child->col_bits = child->row_bits + len;
    child->box_bits = child->row_bits + 2 * len;

    if (!child->row_bits) {
        free(child->grid);
        free(child);
        return NULL;
    }

    // Copy bitmask data
    memcpy(child->row_bits, parent->row_bits, len * sizeof(unsigned long long));
    memcpy(child->col_bits, parent->col_bits, len * sizeof(unsigned long long));
    memcpy(child->box_bits, parent->box_bits, len * sizeof(unsigned long long));

    return child;
}


/**
 * Initializes a new Sudoku puzzle of given base size,
 * and loads puzzle data from file.
 */
Sudoku* init_sudoku(uint8_t N, char *grid_filename) {
    Sudoku* sudoku = malloc(sizeof(Sudoku));
    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    uint8_t len = sudoku->len;

    // Allocate memory for grid
    sudoku->grid = malloc(len * len * sizeof(uint8_t));
    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    // Allocate one block for bitmasks
    sudoku->row_bits = calloc(3 * len, sizeof(unsigned long long));
    sudoku->col_bits = sudoku->row_bits + len;
    sudoku->box_bits = sudoku->row_bits + 2 * len;

    if (!sudoku->row_bits) {
        free(sudoku->grid);
        free(sudoku);
        return NULL;
    }

    // Load grid from file
    populate_grid(sudoku, grid_filename);

    // Initialize bitmasks based on initial grid
    uint8_t r, c, val, box;
    unsigned long long bit;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            val = sudoku->grid[r * len + c];
            if (val > 0) {
                bit = 1ULL << (val - 1);
                sudoku->row_bits[r] |= bit;
                sudoku->col_bits[c] |= bit;
                box = (r / N) * N + (c / N);
                sudoku->box_bits[box] |= bit;
            }
        }
    }

    return sudoku;
}


/**
 * Frees all memory allocated for a Sudoku structure.
 */
void free_sudoku(Sudoku* sudoku) {
    free(sudoku->grid);
    free(sudoku->row_bits);
    free(sudoku);
}


/**
 * Loads puzzle data from a binary file into the Sudoku grid.
 * Expects grid_filename to be a format string for snprintf,
 * e.g. "./grids/sudoku_%dx%d.bin"
 */
static void populate_grid(Sudoku* sudoku, char *grid_filename) {
    char filename[40];
    uint8_t len = sudoku->len;
    uint8_t* grid = sudoku->grid;

    snprintf(filename, sizeof(filename), grid_filename, len, len);
    FILE* file = fopen(filename, "rb");

    if (!file) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Skip first two bytes (dimensions already known)
    fseek(file, 2, SEEK_SET);

    // Read grid data
    unsigned char* data = malloc(len * len);
    if (!data) {
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(data, 1, len * len, file) != (size_t)(len * len)) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    fclose(file);

    for (int i = 0; i < len * len; i++) {
        grid[i] = (uint8_t)data[i];
    }

    free(data);
}


/**
 * Prints the current state of the Sudoku grid to console.
 */
void print_sudoku(Sudoku* sudoku) {
    uint8_t len = sudoku->len;
    int i, j;

    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", sudoku->grid[i * len + j]);
        }
        printf("\n");
    }
}


/**
 * Sets a value in a cell and updates the corresponding bitmasks.
 */
void set_cell(Sudoku* sudoku, uint8_t row, uint8_t col, uint8_t num) {
    sudoku->grid[row * sudoku->len + col] = num;

    unsigned long long bit = 1ULL << (num - 1);
    uint8_t box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    sudoku->row_bits[row] |= bit;
    sudoku->col_bits[col] |= bit;
    sudoku->box_bits[box] |= bit;
}


/**
 * Clears a cell's value and updates the corresponding bitmasks.
 */
void clear_cell(Sudoku* sudoku, uint8_t row, uint8_t col) {
    uint8_t len = sudoku->len;
    uint8_t current_value = sudoku->grid[row * len + col];

    unsigned long long bit = 1ULL << (current_value - 1);
    uint8_t box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    sudoku->row_bits[row] &= ~bit;
    sudoku->col_bits[col] &= ~bit;
    sudoku->box_bits[box] &= ~bit;

    sudoku->grid[row * len + col] = 0;
}


/**
 * Checks whether a completed Sudoku puzzle is valid.
 * Returns 1 if valid, 0 otherwise.
 */
uint8_t is_valid_sudoku(Sudoku* sudoku) {
    size_t i, j;
    uint8_t len = sudoku->len;
    uint8_t base = sudoku->base;

    // Check if all cells are filled
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            if (sudoku->grid[i * len + j] == 0) {
                return 0;
            }
        }
    }

    // Check rows
    for (i = 0; i < len; i++) {
        uint64_t used = 0;
        for (j = 0; j < len; j++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);
            if (used & bit) return 0;
            used |= bit;
        }
    }

    // Check columns
    for (j = 0; j < len; j++) {
        uint64_t used = 0;
        for (i = 0; i < len; i++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);
            if (used & bit) return 0;
            used |= bit;
        }
    }

    // Check boxes
    for (i = 0; i < len; i += base) {
        for (j = 0; j < len; j += base) {
            uint64_t used = 0;
            for (size_t r = 0; r < base; r++) {
                for (size_t c = 0; c < base; c++) {
                    uint8_t num = sudoku->grid[(i + r) * len + (j + c)];
                    uint64_t bit = 1ULL << (num - 1);
                    if (used & bit) return 0;
                    used |= bit;
                }
            }
        }
    }

    return 1;
}

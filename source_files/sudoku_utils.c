#include "../headers/sudoku_types.h"
#include <string.h>
#include <stdint.h>

/* Forward declaration for static functions */
static void populate_grid(Sudoku* sudoku);

/**
* Finds the first empty cell in the Sudoku grid
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
 * Checks if placing a number in a specific cell is valid according to Sudoku rules
 */
uint8_t is_valid_placement(Sudoku* sudoku, uint8_t r, uint8_t c, uint8_t num) {
    unsigned long long bit = 1ULL << (num - 1);
    uint8_t box = (r / sudoku->base) * sudoku->base + (c / sudoku->base);

    // If bit is set in any constraint, placement is invalid
    return !((sudoku->row_bits[r] | sudoku->col_bits[c] | sudoku->box_bits[box]) & bit);

}

/**
 * Creates a deep copy of a Sudoku structure including grid and bitmasks
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

    // Fast copy all cells at once
    memcpy(child->grid, parent->grid, grid_size);

    // One memory allocation for bitmasks
    child->row_bits = malloc(3 * len * sizeof(unsigned long long));
    child->col_bits = child->row_bits + len;
    child->box_bits = child->row_bits + 2 * len;

    if (!child->row_bits) {
        free(child->grid);
        free(child->row_bits);
        free(child);
        return NULL;
    }

    // Copy bitmasks
    memcpy(child->row_bits, parent->row_bits, len * sizeof(unsigned long long));
    memcpy(child->col_bits, parent->col_bits, len * sizeof(unsigned long long));
    memcpy(child->box_bits, parent->box_bits, len * sizeof(unsigned long long));

    return child;
}

/**
 * Initializes a new Sudoku puzzle with given base size and loads puzzle data
 */
Sudoku* init_sudoku(uint8_t N) {
    Sudoku* sudoku = malloc(sizeof(Sudoku));

    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    uint8_t len = N * N;

    // Allocate grid
    sudoku->grid = malloc(len * len * sizeof(uint8_t));

    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    // One memory allocation for bitmasks
    sudoku->row_bits = calloc(3 * len, sizeof(unsigned long long));
    sudoku->col_bits = sudoku->row_bits + len;
    sudoku->box_bits = sudoku->row_bits + 2 * len;

    if (!sudoku->row_bits) {
        free(sudoku->grid);
        free(sudoku->row_bits);
        free(sudoku);
        return NULL;
    }

    populate_grid(sudoku);

    // Initialize bit arrays based on initial grid values
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
 * Frees all memory allocated for a Sudoku structure
 */
void free_sudoku(Sudoku* sudoku) {
    free(sudoku->grid);
    free(sudoku->row_bits);
    free(sudoku);
}

/**
 * Loads puzzle data from a binary file into the Sudoku grid
 */
static void populate_grid(Sudoku* sudoku) {
    char filename[40];
    uint8_t len = sudoku->len;
    uint8_t* grid = sudoku->grid;

    // Assuming placed in ./grids directory
    snprintf(filename, sizeof(filename), "grids/grid_%dx%d.dat", len, len);
    FILE* file = fopen(filename, "rb");

    if (!file) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Skip first two bytes (already known)
    fseek(file, 2, SEEK_SET);

    // Read data from the binary file
    unsigned char* data = malloc(len * len);  // Buffer for faster reading

    if (!data) {
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(data, 1, len * len, file) != (size_t)(len * len)) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    int i;

    for (i = 0; i < len * len; i++) {
        grid[i] = (uint8_t)data[i];
    }

    // Close the file
    free(data);
}

/**
 * Prints the current state of the Sudoku grid to the console
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
 * Sets a value in a cell and updates all corresponding bitmasks
 */
void set_cell(Sudoku* sudoku, uint8_t row, uint8_t col, uint8_t num) {
    // Set the value in the grid
    sudoku->grid[row * sudoku->len + col] = num;

    // Create bitmask with a 1 in the position for this number
    unsigned long long bit = 1ULL << (num - 1);

    // Calculate box index
    uint8_t box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    // Update all three bitmasks
    sudoku->row_bits[row] |= bit;
    sudoku->col_bits[col] |= bit;
    sudoku->box_bits[box] |= bit;
}

/**
 * Clears a cell's value and updates all corresponding bitmasks
 */
void clear_cell(Sudoku* sudoku, uint8_t row, uint8_t col) {
    uint8_t len = sudoku->len;
    uint8_t current_value = sudoku->grid[row * len + col];

    unsigned long long bit = 1ULL << (current_value - 1);

    // Calculate box index
    uint8_t box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    // Clear the bits in all three bitmasks
    sudoku->row_bits[row] &= ~bit;
    sudoku->col_bits[col] &= ~bit;
    sudoku->box_bits[box] &= ~bit;

    sudoku->grid[row * len + col] = 0;
}


/**
 * Checks if a completed Sudoku puzzle is valid
 * Returns 1 if valid, 0 if invalid
 */
uint8_t is_valid_sudoku(Sudoku* sudoku) {
    size_t i, j;
    uint8_t len = sudoku->len;
    uint8_t base = sudoku->base;

    // Check if all cells are filled (no zeros)
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            if (sudoku->grid[i * len + j] == 0) {
                return 0; // Incomplete puzzle
            }
        }
    }

    // Check rows
    for (i = 0; i < len; i++) {
        uint64_t used = 0;

        for (j = 0; j < len; j++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);

            if (used & bit) {
                return 0; // Duplicate in row
            }

            used |= bit;
        }
    }

    // Check columns
    for (j = 0; j < len; j++) {
        uint64_t used = 0;

        for (i = 0; i < len; i++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);

            if (used & bit) {
                return 0; // Duplicate in column
            }

            used |= bit;
        }
    }

    // Check boxes
    for (i = 0; i < len; i += base) {
        for (j = 0; j < len; j += base) {
            uint64_t used = 0;

            // Check each cell in the box
            for (size_t r = 0; r < base; r++) {
                for (size_t c = 0; c < base; c++) {
                    uint8_t num = sudoku->grid[(i + r) * len + (j + c)];
                    uint64_t bit = 1ULL << (num - 1);

                    if (used & bit) {
                        return 0; // Duplicate in box
                    }

                    used |= bit;
                }
            }
        }
    }

    return 1; // Sudoku is valid
}

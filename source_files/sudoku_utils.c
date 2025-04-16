#include "../headers/sudoku_types.h"
#include <string.h>

/* Forward declaration for static functions */
static void populate_board(Sudoku* sudoku);

/**
 * Finds the first empty cell in the Sudoku grid
 */
coord_t first_empty_cell(Sudoku* sudoku) {
    coord_t pos;
    pos.c = 0;
    pos.r = 0;
    pos.found = 0;
    int len = sudoku->len;

    int r, c;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r * len + c].value == 0) {
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
int is_valid_placement(Sudoku* sudoku, int r, int c, int num) {
    unsigned long long bit = 1ULL << (num - 1);
    int box = (r / sudoku->base) * sudoku->base + (c / sudoku->base);

    // If bit is set in any constraint, placement is invalid
    return !(sudoku->row_bits[r] & bit ||
             sudoku->col_bits[c] & bit ||
             sudoku->box_bits[box] & bit);
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
    int len = child->len;

    // Allocate grid memory
    size_t grid_size = len * len * sizeof(Cell);
    child->grid = malloc(grid_size);

    if (!child->grid) {
        free(child);
        return NULL;
    }

    // Fast copy all cells at once
    memcpy(child->grid, parent->grid, grid_size);

    // Allocate and copy bitmasks
    child->row_bits = malloc(len * sizeof(unsigned long long));
    child->col_bits = malloc(len * sizeof(unsigned long long));
    child->box_bits = malloc(len * sizeof(unsigned long long));

    if (!child->row_bits || !child->col_bits || !child->box_bits) {
        free(child->grid);
        free(child->row_bits);
        free(child->col_bits);
        free(child->box_bits);
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
Sudoku* init_sudoku(int N) {
    Sudoku* sudoku = malloc(sizeof(Sudoku));

    if (!sudoku)
        return NULL;

    sudoku->base = N;
    sudoku->len = N * N;
    int len = N * N;

    // Allocate grid
    sudoku->grid = malloc(len * len * sizeof(Cell));

    if (!sudoku->grid) {
        free(sudoku);
        return NULL;
    }

    // Allocate bitmasks
    sudoku->row_bits = calloc(len, sizeof(unsigned long long));
    sudoku->col_bits = calloc(len, sizeof(unsigned long long));
    sudoku->box_bits = calloc(len, sizeof(unsigned long long));

    if (!sudoku->row_bits || !sudoku->col_bits || !sudoku->box_bits) {
        free(sudoku->grid);
        free(sudoku->row_bits);
        free(sudoku->col_bits);
        free(sudoku->box_bits);
        free(sudoku);
        return NULL;
    }

    populate_board(sudoku);

    // Initialize bit arrays based on initial board values
    for (int r = 0; r < len; r++) {
        for (int c = 0; c < len; c++) {
            int val = sudoku->grid[r * len + c].value;

            if (val > 0) {
                unsigned long long bit = 1ULL << (val - 1);
                sudoku->row_bits[r] |= bit;
                sudoku->col_bits[c] |= bit;
                int box = (r / N) * N + (c / N);
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
    free(sudoku->col_bits);
    free(sudoku->box_bits);
    free(sudoku);
}

/**
 * Loads puzzle data from a binary file into the Sudoku grid
 */
static void populate_board(Sudoku* sudoku) {
    char filename[40];
    int len = sudoku->len;
    Cell* grid = sudoku->grid;

    // Assuming placed in ./boards directory
    snprintf(filename, sizeof(filename), "boards/board_%dx%d.dat", len, len);
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

    for (int i = 0; i < len * len; i++) {
        grid[i].value = (int)data[i];
    }

    // Close the file
    free(data);
}

/**
 * Prints the current state of the Sudoku board to the console
 */
void print_sudoku(Sudoku* sudoku) {
    int len = sudoku->len;
    int i, j;

    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", sudoku->grid[i * len + j].value);
        }

        printf("\n");
    }
}

/**
 * Sets a value in a cell and updates all corresponding bitmasks
 */
void set_cell(Sudoku* sudoku, int row, int col, int num) {
    // Set the value in the grid
    sudoku->grid[row * sudoku->len + col].value = num;

    // Create bitmask with a 1 in the position for this number
    unsigned long long bit = 1ULL << (num - 1);

    // Calculate box index
    int box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    // Update all three bitmasks
    sudoku->row_bits[row] |= bit;
    sudoku->col_bits[col] |= bit;
    sudoku->box_bits[box] |= bit;
}

/**
 * Clears a cell's value and updates all corresponding bitmasks
 */
void clear_cell(Sudoku* sudoku, int row, int col) {
    int current_value = sudoku->grid[row * sudoku->len + col].value;

    unsigned long long bit = 1ULL << (current_value - 1);

    // Calculate box index
    int box = (row / sudoku->base) * sudoku->base + (col / sudoku->base);

    // Clear the bits in all three bitmasks
    sudoku->row_bits[row] &= ~bit;
    sudoku->col_bits[col] &= ~bit;
    sudoku->box_bits[box] &= ~bit;

    sudoku->grid[row * sudoku->len + col].value = 0;
}
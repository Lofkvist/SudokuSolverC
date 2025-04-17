#ifndef SUDOKU_UTILS_H
#define SUDOKU_UTILS_H

#include "sudoku_types.h"

/**
 * Find the first empty cell in the Sudoku grid.
 * 
 * @param sudoku Pointer to the Sudoku structure
 * @return A coord_t structure with the coordinates and a found flag
 */
coord_t first_empty_cell(Sudoku* sudoku);

/**
 * Check if placing a number in a specific cell is valid.
 * 
 * @param sudoku Pointer to the Sudoku structure
 * @param r Row index
 * @param c Column index
 * @param num Number to check (1-len)
 * @return 1 if placement is valid, 0 otherwise
 */
 uint8_t is_valid_placement(Sudoku* sudoku, uint8_t r, uint8_t c, uint8_t num);

/**
 * Create a deep copy of a Sudoku structure.
 * 
 * @param parent Pointer to the original Sudoku structure
 * @return Pointer to the new copy or NULL on failure
 */
Sudoku* deep_copy_sudoku(Sudoku* parent);

/**
 * Initialize a Sudoku puzzle with the given base size.
 * Loads the puzzle from a grid file.
 * 
 * @param N The base size of the Sudoku (e.g., 3 for a standard 9x9 puzzle)
 * @return A pointer to the initialized Sudoku structure or NULL on failure
 */
Sudoku* init_sudoku(uint8_t N);

/**
 * Free all memory associated with a Sudoku puzzle.
 * 
 * @param sudoku Pointer to the Sudoku to be freed
 */
void free_sudoku(Sudoku* sudoku);

/**
 * Set a value in a cell and update all corresponding bitmasks.
 * 
 * @param sudoku Pointer to the Sudoku structure
 * @param row Row index
 * @param col Column index
 * @param num Value to set (1-len)
 */
void set_cell(Sudoku* sudoku, uint8_t row, uint8_t col, uint8_t num);

/**
 * Clear a cell's value and update all corresponding bitmasks.
 * 
 * @param sudoku Pointer to the Sudoku structure
 * @param row Row index
 * @param col Column index
 */
void clear_cell(Sudoku* sudoku, uint8_t row, uint8_t col);

/**
 * Print the current state of the Sudoku grid to the console.
 * 
 * @param sudoku Pointer to the Sudoku structure
 */
void print_sudoku(Sudoku *sudoku);

/**
 * Checks if a completed Sudoku puzzle is valid
 *
 * @param sudoku Pointer to the Sudoku structure
 * @return 1 if grid is completly filled and valid, 0 if not
 */
 uint8_t is_valid_sudoku(Sudoku* sudoku);

#endif // SUDOKU_UTILS_H
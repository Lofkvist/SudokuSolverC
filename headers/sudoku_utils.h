// sudoku_utils.h

#ifndef SUDOKU_UTILS_H
#define SUDOKU_UTILS_H

#include "sudoku_types.h"

/**
 * Finds the first empty cell in the Sudoku grid.
 *
 * @param sudoku Pointer to the Sudoku structure
 * @return A coord_t structure containing cell coordinates
 *         and a found flag (found = 1 if empty cell exists)
 */
coord_t first_empty_cell(Sudoku* sudoku);

/**
 * Checks whether placing a number in a specific cell
 * is valid according to Sudoku rules.
 *
 * @param sudoku Pointer to the Sudoku structure
 * @param r Row index
 * @param c Column index
 * @param num Number to check (1 to len)
 * @return 1 if placement is valid, 0 otherwise
 */
uint8_t is_valid_placement(Sudoku* sudoku, uint8_t r, uint8_t c, uint8_t num);

/**
 * Creates a deep copy of a Sudoku structure,
 * duplicating both grid data and bitmasks.
 *
 * @param parent Pointer to the original Sudoku structure
 * @return Pointer to a new copy, or NULL on failure
 */
Sudoku* deep_copy_sudoku(Sudoku* parent);

/**
 * Initializes a new Sudoku puzzle with the given base size.
 * Loads puzzle data from a grid file.
 *
 * @param base Base size of the Sudoku puzzle (e.g. 3 for 9x9)
 * @param grid_filename Path format string for the grid binary file
 * @return Pointer to the initialized Sudoku structure or NULL on failure
 */
Sudoku* init_sudoku(uint8_t base, char *grid_filename);

/**
 * Frees all memory associated with a Sudoku puzzle.
 *
 * @param sudoku Pointer to the Sudoku structure to free
 */
void free_sudoku(Sudoku* sudoku);

/**
 * Sets a value in a specific cell and updates all relevant bitmasks.
 *
 * @param sudoku Pointer to the Sudoku structure
 * @param row Row index
 * @param col Column index
 * @param num Value to set (1 to len)
 */
void set_cell(Sudoku* sudoku, uint8_t row, uint8_t col, uint8_t num);

/**
 * Clears a cell's value and updates all corresponding bitmasks.
 *
 * @param sudoku Pointer to the Sudoku structure
 * @param row Row index
 * @param col Column index
 */
void clear_cell(Sudoku* sudoku, uint8_t row, uint8_t col);

/**
 * Prints the current state of the Sudoku grid to the console.
 *
 * @param sudoku Pointer to the Sudoku structure
 */
void print_sudoku(Sudoku* sudoku);

/**
 * Checks whether a completed Sudoku puzzle is valid.
 *
 * @param sudoku Pointer to the Sudoku structure
 * @return 1 if the grid is completely filled and valid, 0 otherwise
 */
uint8_t is_valid_sudoku(Sudoku* sudoku);

#endif // SUDOKU_UTILS_H

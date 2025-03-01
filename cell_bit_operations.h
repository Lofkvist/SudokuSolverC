#ifndef INIT_SUDOKU_H
#define INIT_SUDOKU_H

#include <stdint.h>
#include "init_sudoku.h"

// Function prototypes

// Gets the bit at the specified position in the candidates bitmask
int get_bit(uint_fast64_t candidates, int val);

// Sets the bit at the specified position in the value bitmask
void set_bit(uint_fast64_t *value, int pos);

// Clears bit at position pos and updates num_candidates if that bit was 1 beforehand
int clear_candidate_bit(Cell* cell, int pos);

// Opposite of above
int set_candidate_bit(Cell* cell, int pos);

int find_iths_set_bit(uint_fast64_t num, int i);

#endif // INIT_SUDOKU_H

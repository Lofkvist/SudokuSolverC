#include "init_sudoku.h"
#include <stdint.h>

// Get bit in position val, index starting at 1
int get_bit(uint_fast64_t candidates, int val) {
    return (candidates >> (val - 1)) & 1;
}

// Set bit at position val to 0 (index starting at 1)
void clear_bit(uint_fast64_t *candidates, int val) {
    *candidates &= ~(1ULL << (val - 1));
}

// Set bit at position val to 1 (index starting at 1)
void set_bit(uint_fast64_t *value, int pos) {
    *value |= (1ULL << (pos - 1));
}

// Remove candidate in position val (index starting at 1).
// Decrement the number of candidates if that cell was 1 beforehand
// Returns 1 if bit was 1 beforehand
int clear_candidate_bit(Cell *cell, int val) {
    int was_set = get_bit(cell->candidates, val);

    if (was_set) {
        cell->num_candidates -= 1;
        // Clear the bit
        cell->candidates &= ~(1ULL << (val - 1));
        return 1; // Return 1 to indicate that the bit was modified
    }

    return 0; // No modification
}

// Set candidate in position val (index starting at 1).
// Increment the number of candidates if that cell was 0 beforehand
// Returns 1 if bit was 0 beforehand
int set_candidate_bit(Cell* cell, int pos) {
    int was_cleared = !get_bit(cell->candidates, pos); // Check if the bit was cleared beforehand

    cell->candidates |= (1ULL << (pos - 1));
    if (was_cleared) {
        cell->num_candidates += 1;
        return 1; // Return 1 to indicate the bit was modified
    }

    return 0; // No modification
}


// Returns the index (index starting at 1) of the first bit
// encountered, starting from 1. Returns -1 of none were found
int find_first_set_bit(int candidates, int len) {
    for (int bit = 1; bit <= len; bit++) {
        if (candidates & (1 << (bit - 1))) {
            return bit; // return number within the valid range (1 - len)
        }
    }
    return -1; // In case no valid candidates are left
}
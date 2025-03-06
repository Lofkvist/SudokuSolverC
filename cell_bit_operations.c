#include "init_sudoku.h"
#include <stdint.h>

int get_bit(uint_fast64_t candidates, int val) {
    return (candidates >> (val - 1)) & 1;
}

void clear_bit(uint_fast64_t *candidates, int val) {
    *candidates &= ~(1ULL << (val - 1));
}

void set_bit(uint_fast64_t *value, int pos) {
    *value |= (1ULL << (pos - 1));
}

// This function removes a candidate and returns 1 if modified, 0 if not
int clear_candidate_bit(Cell *cell, int pos) {
    // Check if the bit was set beforehand
    int was_set = get_bit(cell->candidates, pos);

    if (was_set) {
        // Update num_candidates
        cell->num_candidates -= 1;
        // Clear the bit
        cell->candidates &= ~(1ULL << (pos - 1));
        return 1; // Return 1 to indicate that the bit was modified
    }

    return 0; // No modification
}

// Sets bit at position pos and updates num_candidates if the bit was 0 beforehand
// Returns 1 if the bit was modified, 0 if it wasn't
int set_candidate_bit(Cell* cell, int pos) {
    int was_cleared = !get_bit(cell->candidates, pos); // Check if the bit was cleared beforehand

    cell->candidates |= (1ULL << (pos - 1));
    if (was_cleared) {
        // Set the bit
        // Increment num_candidates
        cell->num_candidates += 1;
        return 1; // Return 1 to indicate the bit was modified
    }

    return 0; // No modification
}


// Returns the position of the first (lowest) set bit
// Returns -1 if no bits are set
int find_first_set_bit(uint_fast64_t num) {
    int position = 1; // Start at position 1 (for Sudoku 1-9)
    
    while (num > 0) {
        if (num & 1) {
            return position; // Found the first set bit
        }
        position++;
        num >>= 1; // Shift right to check next bit
    }
    
    // If we get here, no bits were set
    return -1;
}
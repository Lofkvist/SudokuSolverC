#include "init_sudoku.h"
#include <stdint.h>

int get_bit(uint_fast64_t candidates, int val) {
    return (candidates >> (val - 1)) & 1;
}

void set_bit(uint_fast64_t *value, int pos) {
    *value |= (1ULL << (pos - 1));
}

// This function removes a candidate and returns 1 if modified, 0 if not
int clear_candidate_bit(Cell *cell, int pos) {
    int was_set =
        get_bit(cell->candidates, pos); // Check if the bit was set beforehand

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

    if (was_cleared) {
        // Set the bit
        cell->candidates |= (1ULL << (pos - 1));
        // Increment num_candidates
        cell->num_candidates += 1;
        return 1; // Return 1 to indicate the bit was modified
    }

    return 0; // No modification
}


// Returns the position of the i-th set bit (0-indexed)
// Returns -1 if there are fewer than i+1 set bits
int find_iths_set_bit(uint_fast64_t num, int i) {
    int count = 0;    // Count of set bits found so far
    int position = 0; // Current bit position

    // Iterate through each bit
    while (num > 0) {
        // Check if current bit is set
        if (num & 1) {
            // If this is the i-th set bit, return its position
            if (count == i) {
                return position + 1;
            }
            count++;
        }

        position++;
        num >>= 1; // Shift right to check next bit
    }

    // If we get here, there weren't enough set bits
    return -1;
}
#include <stdint.h>
#include "init_sudoku.h"


int get_bit(uint_fast64_t candidates, int val) {
    return (candidates >> (val-1)) & 1;
}

void set_bit(uint_fast64_t *value, int pos, int len) {
    *value |= (1ULL << (pos - 1));
}

// Clears bit at position pos and updates num_candidates if that bit was 1 beforehand
void update_candidates(Cell* cell, int pos) {
    cell->num_candidates -= get_bit(cell->candidates, pos);
    cell->candidates &= ~(1ULL << (pos - 1));
}
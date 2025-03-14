#include "functions/cell_bit_operations.h"
#include "functions/display_functions.h"
#include "functions/init_sudoku.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
struct timespec ts = {0, 500000000};

/*
Future improvements
- Set numbers directly in init_sudoku instead of in another functions
- Parallelize
- Change to more optimal types (eg uint_fast8_t)
- Place memory allocations closer together? Both the grid itself and all
pointers to peers
- Improve init_cell_peers? Mutual pointers?
- Vectorize all for loops
- Bigger malloc allocations, memory more contigous
*/

/*
- Select the empty cell with the fewest options
- Backtracking solver
*/

typedef struct coord {
    int r;
    int c;
    int found;
} coord_t;

coord_t find_MRV_cell(Sudoku *sudoku);

coord_t first_empty_cell(Sudoku *sudoku);

int is_valid_placement(Sudoku *sudoku, int r, int c, int num);

int is_valid_board(Sudoku *sudoku);

int backtrack(Sudoku *sudoku);

void remove_peer_candidates(Cell *cell, int len);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);

    printf("Before: \n");
    print_sudoku(sudoku);
    printf("\n");
    int solved = backtrack(sudoku);
    printf("After: \n");
    print_sudoku(sudoku);
    printf("Was it solved? %d\n", solved);

    free_sudoku(sudoku);
    return 0;
}

int count_set_bits(uint64_t n, int len) {
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (n & (1ULL << i)) {
            count++;
        }
    }
    return count;
}

int backtrack(Sudoku *sudoku) {
    // 
    coord_t pos = first_empty_cell(sudoku);
    // coord_t pos = find_MRV_cell(sudoku);

    if (pos.found == 0) { // No empty cells found, DONE!
        printf("Klar!\n");
        return 1;
    }

    int r = pos.r;
    int c = pos.c;

    // Save original state
    uint_fast64_t original_candidates = sudoku->grid[r][c].candidates;
    int original_num_cand = sudoku->grid[r][c].num_candidates;

    // Create a temporary copy of candidates to iterate through
    uint_fast64_t temp_candidates = original_candidates;

    // Save peer candidates (to restore later if needed)
    uint_fast64_t original_row_peers[sudoku->len - 1];
    uint_fast64_t original_col_peers[sudoku->len - 1];
    uint_fast64_t original_box_peers[sudoku->len - 1];

    // Save peer candidates (to restore later if needed)
    int original_row_num_cand[sudoku->len - 1];
    int original_col_num_cand[sudoku->len - 1];
    int original_box_num_cand[sudoku->len - 1];

    for (int i = 0; i < sudoku->len - 1; i++) {
        original_row_peers[i] = sudoku->grid[r][c].row_peers[i]->candidates;
        original_col_peers[i] = sudoku->grid[r][c].col_peers[i]->candidates;
        original_box_peers[i] = sudoku->grid[r][c].box_peers[i]->candidates;

        original_row_num_cand[i] =
            sudoku->grid[r][c].row_peers[i]->num_candidates;
        original_col_num_cand[i] =
            sudoku->grid[r][c].col_peers[i]->num_candidates;
        original_box_num_cand[i] =
            sudoku->grid[r][c].box_peers[i]->num_candidates;
    }

    // Try each candidate
    int i;
    for (i = 0; i < original_num_cand; i++) {
        int num = find_first_set_bit(temp_candidates, sudoku->len);
        if (num == -1)
            break;

        // Remove this candidate from our temporary list
        temp_candidates &= ~(1ULL << (num - 1));

        // THIS SHOULD ALWAYS BE TRUE
        if (is_valid_placement(sudoku, r, c, num)) {
            // SET CELL AND SET CANDIDATES = 0
            sudoku->grid[r][c].value = num;
            sudoku->grid[r][c].candidates = 0;
            sudoku->grid[r][c].num_candidates = 0;
            sudoku->unsolved_count--;

            remove_peer_candidates(&sudoku->grid[r][c], sudoku->len);
            /*
            int err = system("clear");
            print_sudoku(sudoku);
            printf("New number set\n");
            nanosleep(&ts, NULL);
            */

            // Try to solve the rest of the board
            if (backtrack(sudoku)) {
                return 1; // Found solution
            }

            // Undo the placement
            sudoku->grid[r][c].value = 0;
            sudoku->unsolved_count++;

            // Mark this candidate as tried in the actual cell's candidates
            sudoku->grid[r][c].candidates = temp_candidates;
            sudoku->grid[r][c].num_candidates =
                count_set_bits(temp_candidates, sudoku->len);

            // Restore the peer candidates (undo the removal)
            for (int j = 0; j < sudoku->len - 1; j++) {
                sudoku->grid[r][c].row_peers[j]->candidates =
                    original_row_peers[j];
                sudoku->grid[r][c].col_peers[j]->candidates =
                    original_col_peers[j];
                sudoku->grid[r][c].box_peers[j]->candidates =
                    original_box_peers[j];

                sudoku->grid[r][c].row_peers[j]->num_candidates =
                    original_row_num_cand[j];
                sudoku->grid[r][c].col_peers[j]->num_candidates =
                    original_col_num_cand[j];
                sudoku->grid[r][c].box_peers[j]->num_candidates =
                    original_box_num_cand[j];
            }

            /*
            int err2 = system("clear");
            print_sudoku(sudoku);
            printf("Backtrack!\n");
            nanosleep(&ts, NULL);
            */
        }
    }

    // If we reach here, none of the candidates worked
    // Restore the original candidate state for parent's backtracking
    sudoku->grid[r][c].candidates = original_candidates;
    sudoku->grid[r][c].num_candidates = original_num_cand;

    return 0; // No solution found
}

coord_t first_empty_cell(Sudoku *sudoku) {
    coord_t pos;
    pos.c = 0;
    pos.r = 0;
    pos.found = 0;
    int len = sudoku->len;

    int r, c;
    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r][c].value == 0) {
                pos.found = 1;
                pos.r = r;
                pos.c = c;
                return pos;
            }
        }
    }
    return pos;
}

coord_t find_MRV_cell(Sudoku *sudoku) {
    coord_t pos;
    pos.c = -1;
    pos.r = -1;
    pos.found = 0;
    int len = sudoku->len;

    int r, c;
    int min_candidates = len;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r][c].value != 0)
                continue;
            if (sudoku->grid[r][c].num_candidates < min_candidates) {
                pos.c = c;
                pos.r = r;
                min_candidates = sudoku->grid[r][c].num_candidates;
                pos.found = 1; // Found one!
            }
        }
    }
    return pos;
}

int is_valid_placement(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;
    int i;

    for (i = 0; i < len - 1; i++) {
        if (num == sudoku->grid[r][c].row_peers[i]->value ||
            num == sudoku->grid[r][c].col_peers[i]->value ||
            num == sudoku->grid[r][c].box_peers[i]->value) {
            return 0; // Invalid placement
        }
    }

    return 1; // Valid placement
}

// Take a cell, and removes the number placed in that cell from peer candidates
void remove_peer_candidates(Cell *cell, int len) {
    int num = cell->value;

    int i;
    for (i = 0; i < len - 1; i++) {
        clear_candidate_bit(cell->box_peers[i], num);
        clear_candidate_bit(cell->row_peers[i], num);
        clear_candidate_bit(cell->col_peers[i], num);
    }
}
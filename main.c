#include "functions/cell_bit_operations.h"
#include "functions/display_functions.h"
#include "functions/init_sudoku.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct timespec ts = {0, 50000000};
struct timespec start, end;

/*
Future improvements
- Set numbers directly in init_sudoku instead of in another functions
- Parallelize
- Change to more optimal types (eg uint_fast8_t)
- Place memory allocations closer together? Both the grid itself and all
pointers to peers
- Improve init_cell_peers? Mutual pointers?
- Vectorize all for loops
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

int total_num_candidates(Sudoku *sudoku);

int fully_solved_board(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    printf("Side length:          %d\n", base*base);


    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    Sudoku *sudoku = init_sudoku(base);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time =
    (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Initialization time: %12.9f seconds\n", elapsed_time);

    clock_gettime(CLOCK_MONOTONIC, &start);
    backtrack(sudoku);
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Solve time:           %.9f seconds\n", elapsed_time);
    printf("Solved correctly? %5d\n", fully_solved_board(sudoku));

    free_sudoku(sudoku);
    return 0;
}

int backtrack(Sudoku *sudoku) {
    //
    coord_t pos = find_MRV_cell(sudoku);
    int len = sudoku->len;

    if (pos.found == 0) { // No empty cells found, DONE!
        return 1;
    }

    int r = pos.r;
    int c = pos.c;

    // Save original state
    uint_fast64_t original_candidates = sudoku->grid[r * len + c].candidates;
    int original_num_cand = sudoku->grid[r * len + c].num_candidates;

    // Create a temporary copy of candidates to iterate through
    uint_fast64_t remaining_candidates = original_candidates;

    // Save peer candidates (to restore later if needed)
    uint_fast64_t original_row_peers[sudoku->len - 1];
    uint_fast64_t original_col_peers[sudoku->len - 1];
    uint_fast64_t original_box_peers[sudoku->len - 1];

    // Save peer candidates (to restore later if needed)
    int original_row_num_cand[sudoku->len - 1];
    int original_col_num_cand[sudoku->len - 1];
    int original_box_num_cand[sudoku->len - 1];

    for (int i = 0; i < sudoku->len - 1; i++) {
        original_row_peers[i] = sudoku->grid[r * len + c].row_peers[i]->candidates;
        original_col_peers[i] = sudoku->grid[r * len + c].col_peers[i]->candidates;
        original_box_peers[i] = sudoku->grid[r * len + c].box_peers[i]->candidates;

        original_row_num_cand[i] = sudoku->grid[r * len + c].row_peers[i]->num_candidates;
        original_col_num_cand[i] = sudoku->grid[r * len + c].col_peers[i]->num_candidates;
        original_box_num_cand[i] = sudoku->grid[r * len + c].box_peers[i]->num_candidates;
    }

    // Try each candidate
    while (remaining_candidates != 0) {
        int num = find_first_set_bit(remaining_candidates, sudoku->len);

        // Remove this candidate from our temporary list
        remaining_candidates &= ~(1ULL << (num - 1));

        // THIS SHOULD ALWAYS BE TRUE
        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell, remove candidates
            sudoku->grid[r * len + c].value = num;
            sudoku->grid[r * len + c].candidates = 0;
            sudoku->grid[r * len + c].num_candidates = 0;

            delete_from_peers(&sudoku->grid[r * len + c], sudoku->len);

            // Try to solve the rest of the board
            if (backtrack(sudoku)) {
                return 1; // Found solution
            }

            // Undo the placement
            sudoku->grid[r * len + c].value = 0;

            // Mark this candidate as tried in the actual cell's candidates
            sudoku->grid[r * len + c].candidates = original_candidates;
            sudoku->grid[r * len + c].num_candidates = original_num_cand;

            // Restore the peer candidates
            for (int j = 0; j < sudoku->len - 1; j++) {
                sudoku->grid[r * len + c].row_peers[j]->candidates =
                    original_row_peers[j];
                sudoku->grid[r * len + c].col_peers[j]->candidates =
                    original_col_peers[j];
                sudoku->grid[r * len + c].box_peers[j]->candidates =
                    original_box_peers[j];

                sudoku->grid[r * len + c].row_peers[j]->num_candidates =
                    original_row_num_cand[j];
                sudoku->grid[r * len + c].col_peers[j]->num_candidates =
                    original_col_num_cand[j];
                sudoku->grid[r * len + c].box_peers[j]->num_candidates =
                    original_box_num_cand[j];
            }
        }
    }

    // Dead end, restore changes
    sudoku->grid[r * len + c].candidates = original_candidates;
    sudoku->grid[r * len + c].num_candidates = original_num_cand;
    return 0;
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
            if (sudoku->grid[r * len + c].value != 0)
                continue;
            if (sudoku->grid[r * len + c].num_candidates < min_candidates) {
                pos.c = c;
                pos.r = r;
                min_candidates = sudoku->grid[r * len + c].num_candidates;
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
        if (num == sudoku->grid[r * len + c].row_peers[i]->value ||
            num == sudoku->grid[r * len + c].col_peers[i]->value ||
            num == sudoku->grid[r * len + c].box_peers[i]->value) {
            return 0; // Invalid placement
        }
    }

    return 1; // Valid placement
}

int total_num_candidates(Sudoku *sudoku) {
    int total = 0;
    int len = sudoku->len;

    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            total += sudoku->grid[i * len + j].num_candidates;
        }
    }
    return total;
}

int fully_solved_board(Sudoku *sudoku) {
    int len = sudoku->len;
    int r, c;
    for (r = 0; r < sudoku->len; r++) {
        for (c = 0; c < sudoku->len; c++) {
            int val = sudoku->grid[r * len + c].value;
            if (val == 0 || !is_valid_placement(sudoku, r, r, val)) {
                return 0; // Either unfilled or invalid cell
            }
        }
    }
    return 1;
}
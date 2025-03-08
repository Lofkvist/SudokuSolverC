#include "functions/cell_bit_operations.h"
#include "functions/display_functions.h"
#include "functions/init_sudoku.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
struct timespec ts = {0, 500000};

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

int claude(Sudoku *sudoku);

int claude2(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);

    print_sudoku(sudoku);
    printf("\n");
    int solved = claude(sudoku);

    print_sudoku(sudoku);
    printf("Solved? %d\n", solved);

    free_sudoku(sudoku);
    return 0;
}
int backtrack(Sudoku *sudoku) {
    coord_t pos = find_MRV_cell(sudoku); // Using MRV heuristic instead
    if (pos.found == 0) {                // No empty cells found, DONE!
        return 1;
    }

    int r = pos.r;
    int c = pos.c;

    // Save original candidates
    uint_fast64_t original_candidates = sudoku->grid[r][c].candidates;
    int original_num_cand = sudoku->grid[r][c].num_candidates;

    // For each candidate
    while (sudoku->grid[r][c].num_candidates > 0) {
        int num =
            find_first_set_bit(sudoku->grid[r][c].candidates, sudoku->len);
        if (num == -1)
            break;

        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell and clear all candidates
            sudoku->grid[r][c].value = num;
            sudoku->grid[r][c].candidates = 0;     // Clear candidates
            sudoku->grid[r][c].num_candidates = 0; // No more candidates
            sudoku->unsolved_count--;

            system("clear");
            nanosleep(&ts, NULL);
            print_sudoku(sudoku);
            int total = 0;
            for (int a = 0; a < sudoku->len; a++) {
                for (int b = 0; b < sudoku->len; b++) {
                    total += sudoku->grid[a][b].num_candidates;
                }
            }
            printf("Total: %d\n", total);

            if (backtrack(sudoku)) {
                return 1; // Found solution
            }

            // Undo the placement
            sudoku->grid[r][c].value = 0;
            sudoku->unsolved_count++;

            // When backtracking, restore candidates except the number we just
            // tried
            sudoku->grid[r][c].candidates = original_candidates;
            sudoku->grid[r][c].num_candidates = original_num_cand;

            // Remove the candidate we just tried (and know doesn't work)
            clear_candidate_bit(&sudoku->grid[r][c], num);
        }
    }

    // No solution for this branch
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

int claude2(Sudoku *sudoku) {
    coord_t pos = first_empty_cell(sudoku);
    if (pos.found == 0) { // No empty cells found, DONE!
        printf("Klar!\n");
        return 1;
    }

    int r = pos.r;
    int c = pos.c;
    uint_fast64_t candidates = sudoku->grid[r][c].candidates;
    uint_fast64_t num_candidates = sudoku->grid[r][c].num_candidates;
    int i;
    // Iterate through candidates using bitwise operations
    for (i = 0; i < num_candidates; i++) {
        int num = find_first_set_bit(
            candidates,
            sudoku->len); // Extract the lowest set bit (1-based index)

        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell
            clear_candidate_bit(&sudoku->grid[r][c], num);
            sudoku->grid[r][c].value = num;
            nanosleep(&ts, NULL);
            print_sudoku(sudoku);

            // Try to solve the rest of the board
            if (claude2(sudoku)) {
                return 1; // Found solution
            }

            // Undo the placement
            sudoku->grid[r][c].value = 0;
        }
    }

    return 0; // No solution found
}

int claude(Sudoku *sudoku) {
    coord_t pos = first_empty_cell(sudoku);

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

    // Try each candidate
    int i;
    for (i = 0; i < temp_candidates; i++) {
        int num = find_first_set_bit(temp_candidates, sudoku->len);
        if (num == -1)
            break;

        // Remove this candidate from our temporary list
        temp_candidates &= ~(1ULL << (num - 1));

        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell
            sudoku->grid[r][c].value = num;
            sudoku->grid[r][c].candidates = 0;
            sudoku->grid[r][c].num_candidates = 0;
            sudoku->unsolved_count--;

            // Try to solve the rest of the board
            if (claude(sudoku)) {
                return 1; // Found solution
            }
            // That route did not work, go back and try a different number in
            // this cell

            // Undo the placement
            sudoku->grid[r][c].value = 0;
            sudoku->unsolved_count++;

            // Mark this candidate as tried in the actual cell's candidates
            sudoku->grid[r][c].candidates =
                temp_candidates; // New candidates with the tested one removed
            sudoku->grid[r][c].num_candidates =
                count_set_bits(temp_candidates, sudoku->len);
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

    printf("cell: (%d,%d) \n", pos.r, pos.c);
    print_sudoku(sudoku);

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
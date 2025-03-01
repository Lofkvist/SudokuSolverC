#include "cell_bit_operations.h"
#include "init_sudoku.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static int size = 0;

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
} coord_t;

void printBinary(uint_fast64_t num, int len);
void print_sudoku(Sudoku *sudoku);

coord_t find_empty_cell(Sudoku *sudoku);
int is_valid_placement(Sudoku *sudoku, int r, int c, int num);

void update_peer_candidates(Sudoku *sudoku, int num, int r, int c);

int solve(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);
    int len = sudoku->len;

    int i, j;
    // printf("%p\n", &sudoku);

    solve(sudoku);

    print_sudoku(sudoku);

    free_sudoku(sudoku);
    return 0;
}

void printBinary(uint_fast64_t num, int len) {
    // Print the bits from the highest bit (63) to the lowest bit (0)
    for (int i = len - 1; i >= 0; i--) {
        putchar((num & (1ULL << i)) ? '1' : '0');
    }
    printf(" ");
}

int solve(Sudoku *sudoku) {
    if (!sudoku->unsolved_count)
        return 1;

    coord_t pos = find_empty_cell(sudoku);
    int r = pos.r;
    int c = pos.c;

    // Temporary variables to store previous states of peers' candidates as
    // bitmasks
    uint64_t prev_box_peers = 0;
    uint64_t prev_row_peers = 0;
    uint64_t prev_col_peers = 0;

    // Try each number in candidates for this cell
    int i, j, k;

    for (i = 0; i < sudoku->grid[r][c].num_candidates; i++) {
        // Get i-th 1 in the candidates bitmask
        int num = find_iths_set_bit(sudoku->grid[r][c].candidates, i);

        if (is_valid_placement(sudoku, r, c, num)) {
            // Place the number
            sudoku->grid[r][c].value = num;
            clear_candidate_bit(&sudoku->grid[r][c], num);
            sudoku->unsolved_count--;

            // Save the previous state of peers and remove num from peers
            // candidates
            for (j = 0; j < sudoku->len - 1; j++) {
                // Save the previous state of the current peer bitmasks
                if (get_bit(sudoku->grid[r][c].box_peers[j]->candidates, num))
                    set_bit(&prev_box_peers, j);

                if (get_bit(sudoku->grid[r][c].row_peers[j]->candidates, num))
                    set_bit(&prev_row_peers, j);

                if (get_bit(sudoku->grid[r][c].col_peers[j]->candidates, num))
                    set_bit(&prev_col_peers, j);

                // Clear the candidate bit
                clear_candidate_bit(sudoku->grid[r][c].box_peers[j], num);
                clear_candidate_bit(sudoku->grid[r][c].row_peers[j], num);
                clear_candidate_bit(sudoku->grid[r][c].col_peers[j], num);
            }


            print_sudoku(sudoku);
            printf("UC: %d\n", sudoku->unsolved_count);
            printf("\n");

            // Recursively try to solve the rest
            if (solve(sudoku)) {
                return 1;
            }


            print_sudoku(sudoku);
            printf("UC: %d\n", sudoku->unsolved_count);
            printf("\n");

            // Backtrack - reset this cell to empty
            sudoku->grid[r][c].value = 0;
            set_candidate_bit(&sudoku->grid[r][c], num);
            sudoku->unsolved_count++;

            // Restore previous states of peers
            for (k = 0; k < sudoku->len - 1; k++) {
                // Restore the previous state if the peer was modified
                if (get_bit(prev_box_peers, k))  // Use k, no +1 needed
                    set_candidate_bit(sudoku->grid[r][c].box_peers[k], num);
                if (get_bit(prev_row_peers, k))  // Use k, no +1 needed
                    set_candidate_bit(sudoku->grid[r][c].row_peers[k], num);
                if (get_bit(prev_col_peers, k))  // Use k, no +1 needed
                    set_candidate_bit(sudoku->grid[r][c].col_peers[k], num);
            }
        }
    }

    return 0; // Dead end
}


// Find cell with fewest candidates
coord_t find_empty_cell(Sudoku *sudoku) {
    coord_t pos;
    pos.c = 0;
    pos.r = 0;
    int len = sudoku->len;

    int r, c;
    int min_candidates = len;

    for (r = 0; r < len; r++) {
        Cell *row = sudoku->grid[r];
        for (c = 0; c < len; c++) {
            if (row[c].num_candidates < min_candidates &&
                row[c].num_candidates > 0) {
                pos.c = c;
                pos.r = r;
                min_candidates = row[c].num_candidates;
            }
        }
    }
    return pos;
}

int is_valid_placement(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;

    int i, j;
    int counter = 0;
    for (i = 0; i < len - 1; i++) {
        counter += (num == sudoku->grid[r][c].box_peers[i]->value);
        counter += (num == sudoku->grid[r][c].row_peers[i]->value);
        counter += (num == sudoku->grid[r][c].col_peers[i]->value);
    }

    return (counter == 0);
}

void print_sudoku(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", grid[i][j].value);
        }
        printf("\n");
    }
}
#include "cell_bit_operations.h"
#include "init_sudoku.h"
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

int is_valid_placement(Sudoku *sudoku, int r, int c, int num);

void print_sudoku(Sudoku *sudoku);

int is_valid_board(Sudoku *sudoku);

void update_peer_candidates(Sudoku *sudoku, int num, int r, int c);

int verify_unsolved_count(Sudoku *sudoku);

void print_candidates(Sudoku *sudoku);

void printBinary(uint_fast64_t num, int len);

void print_num_candidates(Sudoku *sudoku);

int backtrack(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);

    print_sudoku(sudoku);
    printf("\n");
    int solved = backtrack(sudoku);

    print_sudoku(sudoku);
    printf("Solved? %d\n", solved);

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

int backtrack(Sudoku *sudoku) {
    coord_t pos = find_MRV_cell(sudoku);  // Using MRV heuristic instead
    if (pos.found == 0) { // No empty cells found, DONE!
        return 1;
    }
    
    int r = pos.r;
    int c = pos.c;
    
    // Save original candidates
    uint64_t original_candidates = sudoku->grid[r][c].candidates;
    int original_num_cand = sudoku->grid[r][c].num_candidates;
    
    // For each candidate
    while (sudoku->grid[r][c].num_candidates > 0) {
        int num = find_first_set_bit(sudoku->grid[r][c].candidates, sudoku->len);
        if (num == -1) break;
        
        // Remove this candidate
        clear_candidate_bit(&sudoku->grid[r][c], num);
        
        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell and clear all candidates
            sudoku->grid[r][c].value = num;
            sudoku->grid[r][c].candidates = 0;  // Clear all candidates
            sudoku->grid[r][c].num_candidates = 0;  // Set number of candidates to zero
            sudoku->unsolved_count--;
            
            if (backtrack(sudoku)) {
                return 1; // Found solution
            }
            
            // Undo the placement
            sudoku->grid[r][c].value = 0;
            sudoku->unsolved_count++;
            
            // When backtracking, restore candidates except the number we just tried
            sudoku->grid[r][c].candidates = original_candidates;
            sudoku->grid[r][c].num_candidates = original_num_cand;
            
            // Remove the candidate we just tried (and know doesn't work)
            clear_candidate_bit(&sudoku->grid[r][c], num);
        }
    }
    
    // No solution for this branch
    return 0;
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
            if (sudoku->grid[r][c].num_candidates < min_candidates &&
                sudoku->grid[r][c].num_candidates > 0) {
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

void print_candidates(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printBinary(grid[i][j].candidates, len);
        }
        printf("\n");
    }
}

void print_num_candidates(Sudoku *sudoku) {
    int len = sudoku->len;
    Cell **grid = sudoku->grid;
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%2d ", grid[i][j].num_candidates);
        }
        printf("\n");
    }
}

void python_print(Sudoku *sudoku) {
    int len = sudoku->len;

    for (int i = 0; i < len; i++) {
        printf("[");
        for (int j = 0; j < len; j++) {
            printf("%d, ", sudoku->grid[i][j].value);
        }
        printf("],\n");
    }
}
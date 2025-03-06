#include "cell_bit_operations.h"
#include "init_sudoku.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
struct timespec ts = {0, 5000000};
void print_candidates(Sudoku *sudoku);
void printBinary(uint_fast64_t num, int len);
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

void print_sudoku(Sudoku *sudoku);

coord_t find_empty_cell(Sudoku *sudoku);
int is_valid_placement(Sudoku *sudoku, int r, int c, int num);
coord_t first_empty_cell(Sudoku *sudoku);
int is_valid_board(Sudoku *sudoku);

void update_peer_candidates(Sudoku *sudoku, int num, int r, int c);

int verify_unsolved_count(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);

    Sudoku *sudoku = init_sudoku(base);


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
    coord_t pos = find_empty_cell(sudoku);

    if (pos.c == -1) { // No empty cells, SOLUTION FOUND!
        return 1;
    }

    int i;
    int r = pos.r;
    int c = pos.c;

    for (i = 0; i < sudoku->grid[r][c].num_candidates; i++) {
        int num = find_first_set_bit(sudoku->grid[r][c].candidates); // WORKS!

        if (is_valid_placement(sudoku, r, c, num)) {
            // SET CELL
            // UPDATE PEER CANDIDATES

            if (backtrack(sudoku)) { // Now with a new number placed
                return 1;
            }

            // SET CELL BACK TO 0
            // REMOVE THE ATTEMPTED NUMBER FROM THE DOMAIN OF THIS CELL?
            // RESET CELL PEER CANDIDATES
        }
    }
    return 0;
}

coord_t first_empty_cell(Sudoku *sudoku) {
    coord_t pos;
    pos.c = 0;
    pos.r = 0;
    int len = sudoku->len;

    int r, c;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r][c].value == 0) {
                pos.r = r;
                pos.c = c;
                return pos;
            }
        }
    }
    return pos;
}

coord_t find_empty_cell(Sudoku *sudoku) {
    coord_t pos;
    pos.c = -1;
    pos.r = -1;
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
            }
        }
    }

    printf("Selected empty cell at (%d, %d)\n", pos.r, pos.c); // Debug output

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
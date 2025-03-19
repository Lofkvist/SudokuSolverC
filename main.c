
#include "functions/display_functions.h"
#include "functions/parallel.h"
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

int solve(Sudoku *sudoku);

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
    solve(sudoku);
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Solve time:           %.9f seconds\n", elapsed_time);
    printf("Solved correctly? %5d\n", fully_solved_board(sudoku));

    Task* task = create_task(sudoku, 0, 0);

    free_sudoku(sudoku);
    return 0;
}

int solve(Sudoku *sudoku) {
    coord_t pos = first_empty_cell(sudoku);
    int len = sudoku->len;

    if (pos.found == 0) { // No empty cells found, DONE!
        return 1;
    }
    int r = pos.r;
    int c = pos.c;
    // Try each candidate
    int num;
    for (num = 1; num <= sudoku->len; num++) {

        // THIS SHOULD ALWAYS BE TRUE
        if (is_valid_placement(sudoku, r, c, num)) {
            // Set cell
            sudoku->grid[r * len + c].value = num;

            // Backtrack
            if (solve(sudoku)) {
                return 1;
            }
            // Undo the placement
            sudoku->grid[r * len + c].value = 0;
        }
    }

    return 0;
}

/*
void parallel_sudoku_solver(Sudoku *initial_sudoku) {
    int num_threads = get_num_threads();
    WorkDeque deques[num_threads];
    
    // Initialize deques
    for (int i = 0; i < num_threads; i++) {
        init_deque(&deques[i], INIT_CAPACITY);
    }
    
    // Create initial tasks
    for (each valid move) {
        Task *task = create_task(initial_sudoku, row, col);
        push_bottom(&deques[0], task); // Put them in the first worker's deque
    }
    
    // Launch worker threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &deques[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}
*/

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


int fully_solved_board(Sudoku *sudoku) {
    int len = sudoku->len;
    int r, c;
    for (r = 0; r < sudoku->len; r++) {
        for (c = 0; c < sudoku->len; c++) {
            int val = sudoku->grid[r * len + c].value;
            if (val == 0 || !is_valid_placement(sudoku, r, c, val)) {
                return 0; // Either unfilled or invalid cell
            }
        }
    }
    return 1;
}
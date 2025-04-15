
#include "functions/display_functions.h"
#include "functions/init_sudoku.h"
#include "functions/explored_states.h"
#include "functions/parallel.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdatomic.h>
#include <pthread.h>
#include <omp.h>

struct timespec ts = {0, 50000000};
struct timespec start, end;

bool solution_found = false; // Will be accessed using atomic operations
Sudoku* volatile solved_sudoku_atomic = NULL; // Will be accessed using atomic operations


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

// Shared to enable stealing
WorkDeque *deques;
Sudoku *solved_sudoku = NULL; // This will store the solution
ExploredValues* explored_values;

typedef struct {
    Sudoku *sudoku;
    int thread_id;
    int total_threads;
} ThreadArgs;

typedef struct coord {
    int r;
    int c;
    int found;
} coord_t;

coord_t first_empty_cell(Sudoku *sudoku);

int is_valid_placement(Sudoku *sudoku, int r, int c, int num);

int is_valid_board(Sudoku *sudoku);

int solve2(Sudoku *sudoku);

int solve(Sudoku *sudoku, WorkDeque *deque, int depth);

int fully_solved_board(Sudoku *sudoku);

int solve_sequential(Sudoku *sudoku, int depth);
void *solver_thread(void *arg);
int check_row(Sudoku *sudoku, int r, int c, int num);
int check_col(Sudoku *sudoku, int r, int c, int num);
int check_box(Sudoku *sudoku, int r, int c, int num);

void *worker_thread(void *arg);
void parallel_sudoku_solver(Sudoku *initial_sudoku, int N_THREADS);
Task *steal_from_other_deques(int N_THREADS, int thread_id);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    printf("Side length:          %d\n", base * base);

    const int TOTAL_THREADS = 8;
    const int NUM_PTHREADS = TOTAL_THREADS;
    //omp_set_num_threads(0);

    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    Sudoku *sudoku = init_sudoku(base);

    explored_values = init_explored_values(sudoku->len);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Initialization time: %12.9f seconds\n", elapsed_time);

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    print_sudoku(sudoku);
    parallel_sudoku_solver(sudoku, NUM_PTHREADS);
    print_sudoku(sudoku);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (solved_sudoku == NULL) {
        printf("No solution found.\n");
        printf("Solved correctly? No\n");
    } else {
        printf("Solve time: %.9f seconds\n", elapsed_time);
        printf("Solved correctly? %5d\n", fully_solved_board(solved_sudoku));
    }

    free_explored_values(explored_values);
    free_sudoku(sudoku);
    return 0;
}

int solve(Sudoku *sudoku, WorkDeque *deque, int depth) {
    if (__sync_bool_compare_and_swap(&solution_found, false, true)) {
        Sudoku* copied_solution = deep_copy_sudoku(sudoku);
        __sync_synchronize();  // Memory barrier
        solved_sudoku_atomic = copied_solution;
    }

    coord_t pos = first_empty_cell(sudoku);
    if (pos.found == 0) return 1;

    int r = pos.r, c = pos.c, len = sudoku->len;
    int max_parallel_depth = (len <= 16) ? 2 : (len <= 36) ? 3 : 4;

    // Parallel tasks section
    if (depth <= max_parallel_depth) {
        int valid_options[len + 1];
        int valid_count = 0;

        for (int num = 1; num <= len; num++) {
            // Check global explored state
            if (!is_value_tried(explored_values, r, c, num) && is_valid_placement(sudoku, r, c, num)) {
                valid_options[valid_count++] = num;
            }
        }

        if (valid_count > 1) {
            int chunk_size = (depth == 0 && len > 25) ? 2 : 1;

            sudoku->grid[r * len + c].value = valid_options[0];
            mark_value_tried(explored_values, r, c, valid_options[0]);

            for (int i = 1; i < valid_count; i += chunk_size) {
                Task *new_task = create_task(sudoku);
                new_task->sudoku->grid[r * len + c].value = valid_options[i];
                mark_value_tried(explored_values, r, c, valid_options[i]);
                deque_push(deque, new_task);
            }

            if (solve(sudoku, deque, depth + 1)) return 1;
            sudoku->grid[r * len + c].value = 0;
            return 0;
        }
    }

    // Sequential backtracking section
    for (int num = 1; num <= len; num++) {
        // Also check global explored state here
        if (!is_value_tried(explored_values, r, c, num) && is_valid_placement(sudoku, r, c, num)) {
            mark_value_tried(explored_values, r, c, num);
            sudoku->grid[r * len + c].value = num;
            if (solve(sudoku, deque, depth + 1)) return 1;
            sudoku->grid[r * len + c].value = 0;
        }
    }

    return 0;
}

int solve_sequential(Sudoku *sudoku, int depth) {
    if (solution_found) return 0;

    coord_t pos = first_empty_cell(sudoku);
    if (!pos.found) return 1;

    for (int num = 1; num <= sudoku->len; num++) {
        if (!is_value_tried(explored_values, pos.r, pos.c, num) &&
                is_valid_placement(sudoku, pos.r, pos.c, num)) {

            mark_value_tried(explored_values, pos.r, pos.c, num);
            sudoku->grid[pos.r * sudoku->len + pos.c].value = num;

            if (solve_sequential(sudoku, depth + 1)) return 1;

            sudoku->grid[pos.r * sudoku->len + pos.c].value = 0;
        }
    }

    return 0;
}

void parallel_sudoku_solver(Sudoku *initial_sudoku, int N_THREADS) {
    pthread_t threads[N_THREADS];
    ThreadArgs args[N_THREADS];

    // Launch worker threads with different starting positions
    for (int i = 0; i < N_THREADS; i++) {
        args[i].sudoku = deep_copy_sudoku(initial_sudoku);
        args[i].thread_id = i;
        args[i].total_threads = N_THREADS;
        pthread_create(&threads[i], NULL, solver_thread, &args[i]);
    }

    // Join threads
    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Get solution
    solved_sudoku = solved_sudoku_atomic;
}

void *solver_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs*)arg;
    Sudoku *sudoku = args->sudoku;
    int thread_id = args->thread_id;

    // Find empty cells
    coord_t pos = first_empty_cell(sudoku);
    if (pos.found) {
        // Each thread tries different values based on thread ID
        for (int num = 1; num <= sudoku->len; num++) {
            // Skip this value if not for this thread
            if (num % args->total_threads != thread_id) continue;

            // Skip if already tried
            if (is_value_tried(explored_values, pos.r, pos.c, num)) continue;

            if (is_valid_placement(sudoku, pos.r, pos.c, num)) {
                mark_value_tried(explored_values, pos.r, pos.c, num);
                sudoku->grid[pos.r * sudoku->len + pos.c].value = num;

                if (solve_sequential(sudoku, 1)) {
                    if (__sync_bool_compare_and_swap(&solution_found, false, true)) {
                        solved_sudoku_atomic = sudoku;
                    }
                    return NULL;
                }

                sudoku->grid[pos.r * sudoku->len + pos.c].value = 0;
            }
        }
    }

    free_sudoku(sudoku);
    return NULL;
}
Task *steal_from_other_deques(int N_THREADS, int thread_id) {
    // Generate random starting point
    int start = rand() % N_THREADS;
    for (int i = 0; i < N_THREADS - 1; i++) {
        int steal_deque_id = (start + i) % N_THREADS;
        if (steal_deque_id != thread_id) {
            Task *task = deque_steal(&deques[steal_deque_id]);
            if (task) return task;
        }
    }
    return NULL;
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

int is_valid_placement(Sudoku *sudoku, int r, int c, int num) {
    return check_row(sudoku, r, c, num) &&
           check_col(sudoku, r, c, num) &&
           check_box(sudoku, r, c, num);
}

int check_row(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;
    int col;
    int *row = &sudoku->grid[r * len].value;

    for (col = 0; col < c; ++col) {
        if (row[col] == num) {
            return 0; // Found same number in row
        }
    }
    for (col = c + 1; col < len; ++col) {
        if (row[col] == num) {
            return 0; // Found same number in row
        }
    }
    return 1;
}

int check_col(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;

    // Check rows before r
    for (int row = 0; row < r; row++) {
        if (sudoku->grid[row * len + c].value == num) {
            return 0;
        }
    }

    // Check rows after r
    for (int row = r + 1; row < len; row++) {
        if (sudoku->grid[row * len + c].value == num) {
            return 0;
        }
    }

    return 1;
}

int check_box(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;
    int base = sudoku->base;
    int box_row_start = (r / base) * base;
    int box_col_start = (c / base) * base;
    int box_row_end = box_row_start + base;
    int box_col_end = box_col_start + base;
    int row, col;

    for (row = box_row_start; row < box_row_end; row++) {
        for (col = box_col_start; col < box_col_end; col++) {
            // Skip the cell we're checking
            if (row == r && col == c) {
                continue;
            }

            if (sudoku->grid[row * len + col].value == num) {
                return 0;  // Found conflict
            }
        }
    }

    return 1;  // No conflicts
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
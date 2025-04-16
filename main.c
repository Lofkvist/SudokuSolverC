#include "headers/sudoku_utils.h"
#include "headers/queue.h"
#include "headers/parallel_functions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h> // For omp_get_wtime
#include <stdatomic.h> // For atomic operations


// Global shared variables
WorkQueue* work_queue;
Sudoku* solved_sudoku = NULL;
atomic_int solution_found = ATOMIC_VAR_INIT(0);
pthread_mutex_t solution_mutex = PTHREAD_MUTEX_INITIALIZER; // Needed for solved_sudoku


int serial_backtrack(Sudoku* sudoku);

// Main function - just initialize and start the solver
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 6) {
        printf("Usage: %s <BASE> <N_THREADS> <RECURSIVE_DEPTH_LIMIT> <MIN_TASKS_IN_QUEUE> <TASK_BATCH_SIZE>\n",
               argv[0]);
        return 1;
    }

    // Parse command line arguments
    int base = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int depth_limit = atoi(argv[3]);
    int queue_count_minimum = atoi(argv[4]);
    int batch_size = atoi(argv[5]);  // Fixed: was using argv[4] twice

    // Initialize sudoku instances for parallel and serial solving
    Sudoku* sudoku_p = init_sudoku(base);
    Sudoku* sudoku_s = init_sudoku(base);

    // ---------- Parallel Implementation ----------
    double p_start = omp_get_wtime();
    parallel_sudoku_solver(sudoku_p, num_threads, depth_limit, queue_count_minimum, batch_size);
    double p_end = omp_get_wtime();
    double elapsed_p = p_end - p_start;

    if (solved_sudoku == NULL)
        printf("No parallel solution found.\n\n");

    // ---------- Serial Implementation ----------
    double s_start = omp_get_wtime();
    int solved = serial_backtrack(sudoku_s);
    double s_end = omp_get_wtime();
    double elapsed_s = s_end - s_start;

    if (!solved)
        printf("No serial solution found.\n\n");


    // ---------- Performance Comparison ----------
    if (solved && atomic_load(&solution_found)) {
        printf("======== Performance Summary ========\n");
        printf("Parallel: %.6f seconds\n", elapsed_p);
        printf("Serial:   %.6f seconds\n", elapsed_s);
        printf("Speedup:  %.6f \n", elapsed_s / elapsed_p);
        printf("===================================\n");
    }

    // Clean up allocated memory
    free_sudoku(sudoku_p);
    free_sudoku(sudoku_s);

    if (solved_sudoku != NULL && solved_sudoku != sudoku_p) {
        free_sudoku(solved_sudoku);
    }

    return 0;
}


/*
Sequential solver function (standard backtracking)
*/
int serial_backtrack(Sudoku* sudoku) {
    coord_t pos = first_empty_cell(sudoku);

    if (!pos.found)
        return 1;

    for (int num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, pos.r, pos.c, num)) {
            
            
            // Set value
            sudoku->grid[pos.r * sudoku->len + pos.c].value = num;

            // Update bitmasks
            unsigned long long bit = 1ULL << (num - 1);
            int box = (pos.r / sudoku->base) * sudoku->base + (pos.c / sudoku->base);
            sudoku->row_bits[pos.r] |= bit;
            sudoku->col_bits[pos.c] |= bit;
            sudoku->box_bits[box] |= bit;

            if (serial_backtrack(sudoku))
                return 1;

            
                // Backtrack
            sudoku->grid[pos.r * sudoku->len + pos.c].value = 0;

            // Clear the bits
            sudoku->row_bits[pos.r] &= ~bit;
            sudoku->col_bits[pos.c] &= ~bit;
            sudoku->box_bits[box] &= ~bit;
        }
    }

    return 0;
}

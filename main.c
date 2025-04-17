#include "headers/sudoku_utils.h"
#include "headers/queue.h"
#include "headers/parallel_functions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h> // For omp_get_wtime
#include <stdatomic.h> // For atomic operations


// Global shared variables
WorkQueue* work_queue;
Sudoku* solved_sudoku = NULL;
atomic_int solution_found = ATOMIC_VAR_INIT(0);
pthread_mutex_t solution_mutex = PTHREAD_MUTEX_INITIALIZER; // Needed for solved_sudoku

uint8_t is_valid_sudoku(Sudoku* sudoku);
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
    uint8_t base = atoi(argv[1]);
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
        printf("Serial: %.6f seconds\n", elapsed_s);
        printf("Speedup: %.6f \n", elapsed_s / elapsed_p);
        printf("===================================\n");

        // Validate both solutions
        printf("\n======== Solution Validation ========\n");

        // Validate serial solution
        uint8_t serial_valid = is_valid_sudoku(sudoku_s);
        printf("Serial solution: %s\n", serial_valid ? "VALID" : "INVALID");

        // Validate parallel solution
        pthread_mutex_lock(&solution_mutex);
        uint8_t parallel_valid = is_valid_sudoku(solved_sudoku);
        pthread_mutex_unlock(&solution_mutex);
        printf("Parallel solution: %s\n", parallel_valid ? "VALID" : "INVALID");

        // Check if solutions match
        if (serial_valid && parallel_valid) {
            pthread_mutex_lock(&solution_mutex);
            uint8_t solutions_match = 1;

            for (size_t i = 0; i < sudoku_s->len * sudoku_s->len; i++) {
                if (sudoku_s->grid[i] != solved_sudoku->grid[i]) {
                    solutions_match = 0;
                    break;
                }
            }

            pthread_mutex_unlock(&solution_mutex);

            printf("Solutions match: %s\n", solutions_match ? "YES" : "NO");
        }

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
    int num, box;

    if (!pos.found)
        return 1;

    for (num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, pos.r, pos.c, num)) {


            // Set value
            sudoku->grid[pos.r * sudoku->len + pos.c] = num;

            // Update bitmasks
            unsigned long long bit = 1ULL << (num - 1);
            box = (pos.r / sudoku->base) * sudoku->base + (pos.c / sudoku->base);
            sudoku->row_bits[pos.r] |= bit;
            sudoku->col_bits[pos.c] |= bit;
            sudoku->box_bits[box] |= bit;

            if (serial_backtrack(sudoku))
                return 1;


            // Backtrack
            sudoku->grid[pos.r * sudoku->len + pos.c] = 0;

            // Clear the bits
            sudoku->row_bits[pos.r] &= ~bit;
            sudoku->col_bits[pos.c] &= ~bit;
            sudoku->box_bits[box] &= ~bit;
        }
    }

    return 0;
}

/**
 * Checks if a completed Sudoku puzzle is valid
 * Returns 1 if valid, 0 if invalid
 */
uint8_t is_valid_sudoku(Sudoku* sudoku) {
    size_t i, j;
    uint8_t len = sudoku->len;
    uint8_t base = sudoku->base;

    // Check if all cells are filled (no zeros)
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            if (sudoku->grid[i * len + j] == 0) {
                return 0; // Incomplete puzzle
            }
        }
    }

    // Check rows
    for (i = 0; i < len; i++) {
        uint64_t used = 0;

        for (j = 0; j < len; j++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);

            if (used & bit) {
                return 0; // Duplicate in row
            }

            used |= bit;
        }
    }

    // Check columns
    for (j = 0; j < len; j++) {
        uint64_t used = 0;

        for (i = 0; i < len; i++) {
            uint8_t num = sudoku->grid[i * len + j];
            uint64_t bit = 1ULL << (num - 1);

            if (used & bit) {
                return 0; // Duplicate in column
            }

            used |= bit;
        }
    }

    // Check boxes
    for (i = 0; i < len; i += base) {
        for (j = 0; j < len; j += base) {
            uint64_t used = 0;

            // Check each cell in the box
            for (size_t r = 0; r < base; r++) {
                for (size_t c = 0; c < base; c++) {
                    uint8_t num = sudoku->grid[(i + r) * len + (j + c)];
                    uint64_t bit = 1ULL << (num - 1);

                    if (used & bit) {
                        return 0; // Duplicate in box
                    }

                    used |= bit;
                }
            }
        }
    }

    return 1; // Sudoku is valid
}

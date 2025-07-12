#include "headers/sudoku_utils.h"
#include "headers/queue.h"
#include "headers/parallel_functions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h> // For omp_get_wtime
#include <stdatomic.h> // For atomic operations

#define VERBOSE_PERFORMANCE_PRINT 0

// Global shared variables
WorkQueue* work_queue;
Sudoku* solved_sudoku = NULL;
atomic_int solution_found = ATOMIC_VAR_INIT(0);
pthread_mutex_t solution_mutex = PTHREAD_MUTEX_INITIALIZER; // Needed for solved_sudoku

int serial_backtrack(Sudoku* sudoku);

int count_zeros(Sudoku* sudoku) {
    int counter = 0;

    for (int i = 0; i < sudoku->len; i++) {
        for (int j = 0; j < sudoku->len; j++) {
            if (sudoku->grid[i * sudoku->len + j] == 0) {
                counter++;
            }
        }
    }

    return counter;
}

// Main function - just initialize and start the solver
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 6) {
        printf("Usage: %s <BASE> \n"
               "              <FILEPATH> \n"
               "              <NUM_THREADS> \n"
               "              <BASE_DEPTH> \n"
               "              <MIN_TASKS_IN_QUEUE> \n", argv[0]);
        return 1;
    }


    // Parse command line arguments
    uint8_t base = atoi(argv[1]);

    char grid_filename[256];
    strncpy(grid_filename, argv[2], 255);
    grid_filename[255] = '\0'; // Ensure null-termination

    int num_threads = atoi(argv[3]);
    int base_depth = atoi(argv[4]);
    int queue_count_minimum = atoi(argv[5]);

    // Initialize sudoku instances for parallel and serial solving
    Sudoku* sudoku_p = init_sudoku(base, grid_filename);
    Sudoku* sudoku_s = init_sudoku(base, grid_filename);
    double elapsed_s = 0;
    double elapsed_p = 0;
    int serial_solved;


    // ---------- Parallel Implementation ----------
    double p_start = omp_get_wtime();
    parallel_sudoku_solver(sudoku_p,
                           num_threads,
                           base_depth,
                           queue_count_minimum);
    double p_end = omp_get_wtime();
    elapsed_p = p_end - p_start;

    if (solved_sudoku == NULL)
        printf("No parallel solution found.\n\n");


    // ---------- Serial Implementation ----------

    if (VERBOSE_PERFORMANCE_PRINT) {
        double s_start = omp_get_wtime();
        serial_solved = serial_backtrack(sudoku_s);
        double s_end = omp_get_wtime();
        elapsed_s = s_end - s_start;

        if (!serial_solved)
            printf("No serial solution found.\n\n");

    }


    // ---------- Performance Comparison ----------
    if (VERBOSE_PERFORMANCE_PRINT) {
        if (serial_solved && atomic_load(&solution_found)) {
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
    } else {
        printf("%lf\n", elapsed_p);
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
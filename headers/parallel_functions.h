// parallel_functions.h

#ifndef PARALLEL_FUNCTIONS_H
#define PARALLEL_FUNCTIONS_H

#include <pthread.h>
#include <stdatomic.h>
#include "sudoku_types.h"
#include "queue.h"

/**
 * Structure containing arguments passed to each worker thread.
 */
typedef struct {
    int thread_id;             // Thread's unique ID
    int queue_count_minimum;   // Desired minimum size of work queue
    int num_threads;           // Total number of worker threads
    int batch_size;            // Size of task batches (optional, currently unused in some code)
    int base_depth;            // Max recursion depth before backtracking
    int len;                   // Length of one puzzle side (e.g. 9 for 9x9)
} ThreadArg;

/* --------------------------------------------------------
 * Global shared variables for parallel solving
 * -------------------------------------------------------- */

extern WorkQueue* work_queue;
extern Sudoku* solved_sudoku;
extern atomic_int solution_found;

/* --------------------------------------------------------
 * Parallel solving function declarations
 * -------------------------------------------------------- */

/**
 * Creates and manages multiple threads for parallel Sudoku solving.
 *
 * @param sudoku Initial Sudoku grid to solve
 * @param num_threads Number of worker threads to create
 * @param base_depth Maximum recursion depth before switching to backtracking
 * @param queue_count_minimum Minimum number of tasks to maintain in the queue
 */
void parallel_sudoku_solver(Sudoku* sudoku,
                            uint8_t num_threads,
                            uint32_t base_depth,
                            uint32_t queue_count_minimum);

/**
 * Thread function that processes tasks from the shared work queue.
 *
 * @param arg Pointer to a ThreadArg structure
 * @return NULL
 */
void* worker_thread(void* arg);

/**
 * Recursive backtracking algorithm used by worker threads.
 * Checks the global solution status to avoid redundant work.
 *
 * @param sudoku Sudoku grid to solve
 * @return 1 if a solution was found, 0 otherwise
 */
uint8_t worker_backtrack(Sudoku* sudoku);

#endif // PARALLEL_FUNCTIONS_H

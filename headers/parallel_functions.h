#ifndef PARALLEL_FUNCTIONS_H
#define PARALLEL_FUNCTIONS_H

#include <pthread.h>
#include <stdatomic.h>
#include "sudoku_types.h"
#include "queue.h"

// Thread argument structure
typedef struct {
    int thread_id;
    int depth_limit;
    int queue_count_minimum;
    int batch_size;
} ThreadArg;

// Global variables
extern WorkQueue* work_queue;
extern Sudoku* solved_sudoku;
extern atomic_int solution_found;

/**
 * Creates and manages multiple threads for parallel sudoku solving
 *
 * @param sudoku Initial sudoku board to solve
 * @param num_threads Number of worker threads to create
 * @param depth_limit Maximum recursion depth before switching to backtracking
 * @param queue_count_minimum Minimum number of tasks to maintain in queue
 * @param batch_size Size of task batches for queue operations
 */
void parallel_sudoku_solver(Sudoku* sudoku,
    int num_threads,
    int depth_limit,
    int queue_count_minimum,
    int batch_size);

/**
 * Thread function that processes tasks from the shared work queue
 *
 * @param arg Pointer to ThreadArg structure
 * @return NULL
 */
void* worker_thread(void* arg);

/**
 * Recursive backtracking algorithm used by worker threads
 * Checks global solution status to avoid redundant work
 *
 * @param sudoku Sudoku board to solve
 * @return 1 if solution found, 0 otherwise
 */
 uint8_t worker_backtrack(Sudoku* sudoku);

#endif // PARALLEL_FUNCTIONS_H
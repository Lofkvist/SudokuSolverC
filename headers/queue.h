// queue.h
#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "sudoku_types.h"

/**
 * Represents a single task to be processed,
 * containing a pointer to a Sudoku puzzle state
 * and the current search depth.
 */
typedef struct {
    Sudoku* grid;
    int depth;
} Task;

/**
 * Thread-safe circular work queue structure.
 * Used for sharing tasks between worker threads.
 */
typedef struct {
    Task** tasks;              // Array of task pointers
    uint32_t capacity;         // Max number of tasks
    uint32_t size;             // Current number of tasks
    uint32_t head;             // Index of next task to pop
    uint32_t tail;             // Index for inserting new tasks
    pthread_mutex_t lock;      // Mutex for thread safety
    pthread_cond_t not_empty;  // Condition variable for wait/signal
} WorkQueue;

/* --------------------------------------------------------
 * External shared variables for parallel solving
 * -------------------------------------------------------- */

extern WorkQueue* work_queue;
extern Sudoku* solved_sudoku;
extern atomic_int solution_found;
extern pthread_mutex_t solution_mutex;

/* --------------------------------------------------------
 * Work queue function declarations
 * -------------------------------------------------------- */

/**
 * Creates a new work queue with the specified capacity.
 *
 * @param capacity Maximum number of tasks the queue can hold
 * @return Pointer to the newly created WorkQueue
 */
WorkQueue* create_work_queue(uint32_t capacity);

/**
 * Pops a task from the front of the queue.
 * Blocks if the queue is empty until a task is available
 * or a solution has been found.
 *
 * @param queue Pointer to the work queue
 * @return Pointer to the popped Task, or NULL if interrupted
 */
Task* queue_pop(WorkQueue* queue);

/**
 * Pushes a batch of tasks into the queue in a single operation.
 * Any tasks that do not fit due to capacity limits are freed.
 *
 * @param queue Pointer to the work queue
 * @param tasks Array of pointers to Task structures
 * @param count Number of tasks to add
 */
void queue_push_batch(WorkQueue* queue, Task** tasks, uint32_t count);

/**
 * Returns the current number of tasks in the queue.
 *
 * @param queue Pointer to the work queue
 * @return Number of tasks in the queue
 */
uint32_t queue_get_size(WorkQueue* queue);

/**
 * Checks whether the number of tasks in the queue
 * is below a specified threshold.
 *
 * @param queue Pointer to the work queue
 * @param threshold Minimum desired number of tasks
 * @return 1 if queue size is below threshold, 0 otherwise
 */
uint8_t queue_is_low(WorkQueue* queue, uint32_t threshold);

#endif /* WORK_QUEUE_H */

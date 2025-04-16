#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "sudoku_types.h"

// Task structure
typedef struct {
    Sudoku* board;
    int depth;
} Task;

// Thread-safe work queue
typedef struct {
    Task** tasks;
    int capacity;
    int size;
    int head;
    int tail;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} WorkQueue;

// External variables
extern WorkQueue* work_queue;
extern Sudoku* solved_sudoku;
extern atomic_int solution_found;
extern pthread_mutex_t solution_mutex;

/**
 * Creates a new work queue with specified capacity
 * 
 * @param capacity Maximum number of tasks the queue can hold
 * @return Pointer to the new queue
 */
WorkQueue* create_work_queue(int capacity);

/**
 * Pops a task from the front of the queue
 * 
 * @param queue Pointer to the work queue
 * @return Pointer to the task or NULL if interrupted
 */
Task* queue_pop(WorkQueue* queue);

/**
 * Pushes a batch of tasks to the queue at once
 * 
 * @param queue Pointer to the work queue
 * @param tasks Array of task pointers
 * @param count Number of tasks in the batch
 */
void queue_push_batch(WorkQueue* queue, Task** tasks, int count);

/**
 * Checks if the queue size is below a given threshold
 * 
 * @param queue Pointer to the work queue
 * @param threshold Minimum desired number of tasks
 * @return 1 if queue size is below threshold, 0 otherwise
 */
int queue_is_low(WorkQueue* queue, int threshold);

#endif /* WORK_QUEUE_H */
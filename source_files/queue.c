/* --------------------------------------------------------
 * File:        queue.c
 * Author:      Carl Löfkvist
 * Date:        2025-07-13
 * Description: Function definitions for the task queue used by
 *              the parallel solver
 * -------------------------------------------------------- */

#include <pthread.h>
#include <stdlib.h>
#include "../headers/queue.h"
#include "../headers/sudoku_utils.h"
#include <stdatomic.h>

/**
 * Creates a new work queue with specified capacity
 */
WorkQueue* create_work_queue(uint32_t capacity) {
    WorkQueue* queue = malloc(sizeof(WorkQueue));
    queue->tasks = malloc(sizeof(Task*) * capacity);
    queue->capacity = capacity;
    queue->size = 0;
    queue->head = 0;
    queue->tail = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    return queue;
}

/**
 * Removes and returns a task from the queue
 * If the queue is empty, waits until a task is available
 * or until a solution is found.
 */
Task* queue_pop(WorkQueue* queue) {
    Task* task = NULL;

    // Check if solution is already found
    if (atomic_load(&solution_found))
        return NULL;

    pthread_mutex_lock(&queue->lock);

    // Wait while queue is empty
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);

        // Check again if solution was found while waiting
        if (atomic_load(&solution_found)) {
            pthread_mutex_unlock(&queue->lock);
            return NULL;
        }
    }

    // Retrieve task from queue
    task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->lock);
    return task;
}

/**
 * Adds multiple tasks to the queue in a single operation.
 * If the queue cannot fit all tasks, extra tasks are freed.
 */
void queue_push_batch(WorkQueue* queue, Task** tasks, uint32_t count) {
    pthread_mutex_lock(&queue->lock);

    uint8_t added = 0;
    int i;

    for (i = 0; i < count && queue->size < queue->capacity; i++) {
        queue->tasks[queue->tail] = tasks[i];
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->size++;
        added++;
    }

    // Signal waiting threads if queue was previously empty
    if (added > 0 && queue->size - added == 0) {
        pthread_cond_broadcast(&queue->not_empty);
    }

    pthread_mutex_unlock(&queue->lock);

    // Free tasks that didn't fit into the queue
    for (i = added; i < count; i++) {
        free_sudoku(tasks[i]->grid);
        free(tasks[i]);
    }
}

/**
 * Returns the current number of tasks in the queue
 */
uint32_t queue_get_size(WorkQueue* queue) {
    uint32_t size;
    pthread_mutex_lock(&queue->lock);
    size = queue->size;
    pthread_mutex_unlock(&queue->lock);
    return size;
}

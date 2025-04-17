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
 */
Task* queue_pop(WorkQueue* queue) {
    Task* task = NULL;
    
    // Check if solution is found
    if (atomic_load(&solution_found))
        return NULL;
        
    pthread_mutex_lock(&queue->lock);
    
    // Wait for tasks if queue is empty
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
        
        // Check if solution was found while waiting
        if (atomic_load(&solution_found)) {
            pthread_mutex_unlock(&queue->lock);
            return NULL;
        }
    }
    
    // Get task from queue
    task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;
    
    pthread_mutex_unlock(&queue->lock);
    return task;
}

/**
 * Adds multiple tasks to the queue in a single operation
 */
void queue_push_batch(WorkQueue* queue, Task** tasks, uint32_t count) {
    pthread_mutex_lock(&queue->lock);
    
    // Add tasks to queue
    uint8_t added = 0;
    int i;
    for (i = 0; i < count && queue->size < queue->capacity; i++) {
        queue->tasks[queue->tail] = tasks[i];
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->size++;
        added++;
    }
    
    // Signal waiting threads if queue was empty
    if (added > 0 && queue->size - added == 0) {
        pthread_cond_broadcast(&queue->not_empty);
    }
    
    pthread_mutex_unlock(&queue->lock);
    
    // Clean up tasks that didn't fit
    for (i = added; i < count; i++) {
        free_sudoku(tasks[i]->board);
        free(tasks[i]);
    }
}

/**
 * Checks if the queue size is below a given threshold
 */
 uint8_t queue_is_low(WorkQueue* queue, uint32_t threshold) {
    uint32_t size;
    pthread_mutex_lock(&queue->lock);
    size = queue->size;
    pthread_mutex_unlock(&queue->lock);
    return size < threshold;
}
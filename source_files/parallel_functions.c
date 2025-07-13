/* --------------------------------------------------------
 * File:        parallel_functions.c
 * Author:      Carl Löfkvist
 * Date:        2025-07-13
 * Description: Function definitions used by the parallel solver
 * -------------------------------------------------------- */

#include "../headers/parallel_functions.h"
#include "../headers/sudoku_types.h"
#include "../headers/sudoku_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdatomic.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

/**
 * Creates and manages multiple threads for parallel sudoku solving
 */
void parallel_sudoku_solver(Sudoku* sudoku,
                            uint8_t num_threads,
                            uint32_t base_depth,
                            uint32_t queue_count_minimum) 
{
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    // Create work queue
    const int queue_max_capacity = 1000 * queue_count_minimum;
    work_queue = create_work_queue(queue_max_capacity);

    // Prepare the initial task containing the original puzzle
    Task* initial_task = malloc(sizeof(Task));
    initial_task->grid = sudoku;
    initial_task->depth = 0;

    Task* tasks[] = { initial_task };
    queue_push_batch(work_queue, tasks, 1);

    // Create worker threads
    int i;
    for (i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].queue_count_minimum = queue_count_minimum;
        args[i].num_threads = num_threads;
        args[i].base_depth = base_depth;
        args[i].len = sudoku->len;

        pthread_create(&threads[i], NULL, worker_thread, &args[i]);
    }

    // Wait for all threads to finish
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up the work queue
    free(work_queue->tasks);
    pthread_cond_destroy(&work_queue->not_empty);
    free(work_queue);
}


/**
 * Thread function that processes tasks from the shared work queue
 */
void* worker_thread(void* arg) 
{
    coord_t cell;
    uint16_t row, col;
    uint16_t num;

    ThreadArg* thread_arg = (ThreadArg*)arg;

    const int base_batch_size = 10;
    const int batch_size = MAX(base_batch_size * thread_arg->len,
                               thread_arg->queue_count_minimum / thread_arg->num_threads);

    while (1) {

        // Check if solution was already found by another thread
        if (atomic_load(&solution_found))
            break;

        // Get next task from the queue
        Task* task = queue_pop(work_queue);

        if (task == NULL) {
            continue;
        }

        // Get current queue size
        int current_queue_size = queue_get_size(work_queue);

        /**
         * If the queue is small and we're still at a shallow depth,
         * expand the search tree by generating new tasks
         */
        if (current_queue_size < thread_arg->queue_count_minimum &&
            task->depth < thread_arg->base_depth) 
        {
            cell = first_empty_cell(task->grid);
            row = cell.r;
            col = cell.c;

            if (cell.found) {
                // Prepare a batch of new tasks
                Task* batch[batch_size];
                int batch_count = 0;

                for (num = 1; num <= task->grid->len; num++) {
                    if (is_valid_placement(task->grid, row, col, num)) {
                        Sudoku* new_grid = deep_copy_sudoku(task->grid);
                        set_cell(new_grid, row, col, num);

                        Task* new_task = malloc(sizeof(Task));
                        new_task->grid = new_grid;
                        new_task->depth = task->depth + 1;

                        batch[batch_count++] = new_task;

                        // Push batch if full
                        if (batch_count == batch_size) {
                            queue_push_batch(work_queue, batch, batch_count);
                            batch_count = 0;
                        }
                    }
                }

                // Push any remaining tasks that didn’t fill a full batch
                if (batch_count > 0) {
                    queue_push_batch(work_queue, batch, batch_count);
                }
            }
        } 
        else 
        {
            // Try to solve the puzzle directly via backtracking
            if (worker_backtrack(task->grid)) {
                int expected = 0;

                if (atomic_compare_exchange_strong(&solution_found, &expected, 1)) {
                    // First thread to find solution saves it
                    solved_sudoku = deep_copy_sudoku(task->grid);

                    // Wake all waiting threads so they can exit
                    pthread_mutex_lock(&work_queue->lock);
                    pthread_cond_broadcast(&work_queue->not_empty);
                    pthread_mutex_unlock(&work_queue->lock);
                }
            }
        }

        // Free memory allocated for tasks
        if (task->depth > 0) {
            free_sudoku(task->grid);
        }

        free(task);
    }

    return NULL;
}


/**
 * Recursive backtracking algorithm used by worker threads
 * Returns 1 if solved, 0 otherwise
 */
uint8_t worker_backtrack(Sudoku* sudoku) 
{
    coord_t cell = first_empty_cell(sudoku);

    // If no empty cell is left, puzzle is solved
    if (!cell.found) {
        return 1;
    }

    uint8_t row = cell.r;
    uint8_t col = cell.c;
    uint8_t num;

    for (num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, row, col, num)) {
            set_cell(sudoku, row, col, num);

            if (worker_backtrack(sudoku)) {
                return 1;
            }

            // Backtrack
            clear_cell(sudoku, row, col);
        }
    }

    return 0;
}

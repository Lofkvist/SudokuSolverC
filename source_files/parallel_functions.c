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
                            uint32_t depth_limit,
                            uint32_t queue_count_minimum,
                            uint32_t batch_size) {
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    // Create work queue
    work_queue = create_work_queue(1000 * depth_limit);

    int i;

    Task* initial_task = malloc(sizeof(Task));
    initial_task->grid = sudoku;
    initial_task->depth = 0;

    Task* tasks[] = { initial_task };
    queue_push_batch(work_queue, tasks, 1);



    // Create worker threads
    for (i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].depth_limit = depth_limit;
        args[i].queue_count_minimum = queue_count_minimum;
        args[i].batch_size = batch_size;
        args[i].num_threads = num_threads;
        pthread_create(&threads[i], NULL, worker_thread, &args[i]);
    }

    // Wait for all threads to finish
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    free(work_queue->tasks);
    pthread_cond_destroy(&work_queue->not_empty);
    free(work_queue);
}

/**
 * Thread function that processes tasks from the shared work queue
 */

void* worker_thread(void* arg) {
    coord_t cell;
    uint16_t row, col;
    uint16_t num;
    ThreadArg* thread_arg = (ThreadArg*)arg;

    while (1) {
        // Check if solution was already found
        if (atomic_load(&solution_found))
            break;

        // Get task from queue
        Task* task = queue_pop(work_queue);


        if (task == NULL) {
            continue;
        }

        if (task->depth < thread_arg->depth_limit
                || queue_is_low(work_queue, thread_arg->queue_count_minimum)) {
            cell = first_empty_cell(task->grid);
            row = cell.r;
            col = cell.c;

            if (cell.found) {
                // Generate and queue new tasks
                Task* batch[thread_arg->batch_size];
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
                        if (batch_count == thread_arg->batch_size) {
                            queue_push_batch(work_queue, batch, batch_count);
                            batch_count = 0;
                        }
                    }
                }

                // Push any remaining tasks
                if (batch_count > 0) {
                    queue_push_batch(work_queue, batch, batch_count);
                }
            }
        } else {
            // Try backtracking solution
            if (worker_backtrack(task->grid)) {
                int expected = 0;

                if (atomic_compare_exchange_strong(&solution_found, &expected, 1)) {
                    // Save solution and wake waiting threads
                    solved_sudoku = deep_copy_sudoku(task->grid);

                    pthread_mutex_lock(&work_queue->lock);
                    pthread_cond_broadcast(&work_queue->not_empty);
                    pthread_mutex_unlock(&work_queue->lock);
                }
            }
        }

        // Clean up
        if (task->depth > 0) {
            free_sudoku(task->grid);
        }

        free(task);
    }

    return NULL;
}

/**
 * Recursive backtracking algorithm used by worker threads
 */// Modify the worker_backtrack function to accept and track depth
uint8_t worker_backtrack(Sudoku* sudoku) {
    coord_t cell = first_empty_cell(sudoku);

    // If no empty cell found, puzzle is solved
    if (!cell.found) {
        return 1;
    }

    uint8_t row = cell.r;
    uint8_t col = cell.c;
    uint8_t num;

    // Try each possible number in this cell
    for (num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, row, col, num)) {
            // Place the number and increment depth
            set_cell(sudoku, row, col, num);

            // Recursively solve the rest
            if (worker_backtrack(sudoku)) {
                return 1;
            }

            // If not solved, backtrack
            clear_cell(sudoku, row, col);
        }
    }

    return 0;
}
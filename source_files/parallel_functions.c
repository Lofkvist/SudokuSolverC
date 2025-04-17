#include "../headers/parallel_functions.h"
#include "../headers/sudoku_types.h"
#include "../headers/sudoku_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>


/**
 * Creates and manages multiple threads for parallel sudoku solving
 */
void parallel_sudoku_solver(Sudoku* sudoku,
                            int num_threads,
                            int depth_limit,
                            int queue_count_minimum,
                            int batch_size) {
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    // Create work queue
    work_queue = create_work_queue(1000 * depth_limit);

    // Create initial task and add to queue
    Task* initial_task = malloc(sizeof(Task));
    initial_task->board = sudoku;
    initial_task->depth = 0;

    Task* batch[1] = {initial_task};
    queue_push_batch(work_queue, batch, 1);

    // Create worker threads
    int i;

    for (i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].depth_limit = depth_limit;
        args[i].queue_count_minimum = queue_count_minimum;
        args[i].batch_size = batch_size;
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

        if (task->depth < thread_arg->depth_limit || queue_is_low(work_queue, thread_arg->queue_count_minimum)) {
            cell = first_empty_cell(task->board);
            row = cell.r;
            col = cell.c;

            if (cell.found) {
                // Generate and queue new tasks
                Task* batch[thread_arg->batch_size];
                int batch_count = 0;

                for (num = 1; num <= task->board->len; num++) {
                    if (is_valid_placement(task->board, row, col, num)) {
                        Sudoku* new_board = deep_copy_sudoku(task->board);
                        set_cell(new_board, row, col, num);

                        Task* new_task = malloc(sizeof(Task));
                        new_task->board = new_board;
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
            if (worker_backtrack(task->board)) {
                int expected = 0;

                if (atomic_compare_exchange_strong(&solution_found, &expected, 1)) {
                    // Save solution and wake waiting threads
                    solved_sudoku = deep_copy_sudoku(task->board);

                    pthread_mutex_lock(&work_queue->lock);
                    pthread_cond_broadcast(&work_queue->not_empty);
                    pthread_mutex_unlock(&work_queue->lock);
                }
            }
        }

        // Clean up
        if (task->depth > 0) {
            free_sudoku(task->board);
        }

        free(task);
    }

    return NULL;
}

/**
 * Recursive backtracking algorithm used by worker threads
 */
uint8_t worker_backtrack(Sudoku* sudoku) {
    // Exit if solution already found
    if (atomic_load(&solution_found))
        return 0;

    coord_t pos = first_empty_cell(sudoku);

    if (!pos.found)
        return 1;  // Solution found!

    int num;

    for (num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, pos.r, pos.c, num)) {
            set_cell(sudoku, pos.r, pos.c, num);

            if (worker_backtrack(sudoku))
                return 1;

            clear_cell(sudoku, pos.r, pos.c);
        }
    }

    return 0;
}
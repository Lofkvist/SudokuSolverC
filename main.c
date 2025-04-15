#include "functions/display_functions.h"
#include "functions/explored_states.h"
#include "functions/init_sudoku.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h> // For omp_get_wtime

// Global variables for sharing work and solution
typedef struct {
    Sudoku* board;
    int depth;
} Task;

typedef struct coord {
    int r;
    int c;
    int found;
} coord_t;

// Simple thread-safe work queue
typedef struct {
    Task** tasks;
    int capacity;
    int size;
    int head;
    int tail;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;  // New condition variable
} WorkQueue;

// Global shared variables
WorkQueue* work_queue;
Sudoku* solved_sudoku = NULL;
int solution_found = 0;
pthread_mutex_t solution_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread argument structure
typedef struct {
    int thread_id;
    int depth_limit;
} ThreadArg;

// Function declarations
void parallel_sudoku_solver(Sudoku* sudoku, int num_threads, int depth_limit);
void* worker_thread(void* arg);
int solve_sequential(Sudoku* sudoku);
int serial_solve(Sudoku* sudoku);
coord_t first_empty_cell(Sudoku* sudoku);
Sudoku* deep_copy_sudoku(Sudoku* parent);

int is_valid_placement(Sudoku* sudoku, int row, int col, int num);
int check_row(Sudoku* sudoku, int r, int c, int num);
int check_col(Sudoku* sudoku, int r, int c, int num);
int check_box(Sudoku* sudoku, int r, int c, int num);

// Main function - just initialize and start the solver

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <BASE> <NUM_THREADS> <BACKTRACK_RECURSIVE_DEPTH_LIMIT>\n", argv[0]);
        return 1;
    }

    
    int base = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int depth_limit = atoi(argv[3]);

    
    // Initialize sudoku
    Sudoku* sudoku1 = init_sudoku(base);
    Sudoku* sudoku2 = init_sudoku(base);

    // Solve using parallel approach
    double p_start, p_end, s_start, s_end;

    p_start = omp_get_wtime();
    parallel_sudoku_solver(sudoku1, num_threads, depth_limit);
    p_end = omp_get_wtime();

    s_start = omp_get_wtime();
    int solved = serial_solve(sudoku2);
    s_end = omp_get_wtime();


    double elapsed_p = p_end - p_start;
    printf("Parallel implementation: \n");

    if (solved_sudoku != NULL) {
        printf("Execution time:  %lf \n", p_end - p_start);
        printf("------------------ \n");
    } else {
        printf("No solution found.\n");
    }

    double elapsed_s = s_end - s_start;
    printf("------------------ \n");
    printf("Serial implementation: \n");

    if (solved) {
        printf("Execution time:  %lf \n", s_end - s_start);
        printf("------------------ \n");
    } else {
        printf("No solution found.\n");
        printf("------------------ \n");
    }

    if (solved && solution_found) {
        printf("Speedup: %lf\n", elapsed_s / elapsed_p);
    }


    // Cleanup
    free_sudoku(sudoku1);
    free_sudoku(sudoku2);

    if (solved_sudoku != NULL && solved_sudoku != sudoku1) {
        free_sudoku(solved_sudoku);
    }

    return 0;
}

// Initialize work queue
WorkQueue* create_work_queue(int capacity) {
    WorkQueue* queue = malloc(sizeof(WorkQueue));
    queue->tasks = malloc(sizeof(Task*) * capacity);
    queue->capacity = capacity;
    queue->size = 0;
    queue->head = 0;
    queue->tail = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty, NULL);  // Initialize condition
    return queue;
}

// Add task to work queue
int queue_push(WorkQueue* queue, Task* task) {
    int result = 0;
    pthread_mutex_lock(&queue->lock);

    if (queue->size < queue->capacity) {
        queue->tasks[queue->tail] = task;
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->size++;
        result = 1;

        if (queue->size == 1) {  // Queue was empty, now has an item
            pthread_cond_broadcast(&queue->not_empty);  // Wake up waiting threads
        }
    }

    pthread_mutex_unlock(&queue->lock);
    return result;
}

Task* queue_pop(WorkQueue* queue) {
    Task* task = NULL;
    int found = 0;

    // First check if solution is found (always check solution_mutex first)
    pthread_mutex_lock(&solution_mutex);
    found = solution_found;
    pthread_mutex_unlock(&solution_mutex);

    if (found)
        return NULL;

    // Then acquire queue lock
    pthread_mutex_lock(&queue->lock);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);

        // Check if solution was found while waiting
        pthread_mutex_unlock(&queue->lock);
        pthread_mutex_lock(&solution_mutex);
        found = solution_found;
        pthread_mutex_unlock(&solution_mutex);

        if (found) {
            return NULL;
        }

        pthread_mutex_lock(&queue->lock);
    }

    // Pop task from queue
    task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->lock);
    return task;
}


// Main solver function that creates threads and distributes work
void parallel_sudoku_solver(Sudoku* sudoku, int num_threads, int depth_limit) {
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    // Create work queue
    work_queue = create_work_queue(1000);

    // Create initial task and add to queue
    Task* initial_task = malloc(sizeof(Task));
    initial_task->board = sudoku;
    initial_task->depth = 0;
    queue_push(work_queue, initial_task);

    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].depth_limit = depth_limit;
        pthread_create(&threads[i], NULL, worker_thread, &args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    free(work_queue->tasks);
    // After joining threads:
    pthread_cond_destroy(&work_queue->not_empty);
    free(work_queue);
}

// Worker thread function
void* worker_thread(void* arg) {
    coord_t cell;
    int row, col;
    ThreadArg* thread_arg = (ThreadArg*)arg;
    int thread_id = thread_arg->thread_id;


    // Per-thread activity counters
    long tasks_processed = 0;
    long idle_cycles = 0;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000;  // 100 microseconds = 100,000 nanoseconds

    while (1) {
        // Check if solution was already found
        pthread_mutex_lock(&solution_mutex);
        int found = solution_found;
        pthread_mutex_unlock(&solution_mutex);

        if (found)
            break;

        // Get task from queue (will wait efficiently if queue is empty)
        Task* task = queue_pop(work_queue);

        if (task == NULL) {
            idle_cycles++;
            nanosleep(&ts, NULL);
            continue;
        }

        // Task was found - record active time
        tasks_processed++;

        if (task->depth < thread_arg->depth_limit) {
            cell = first_empty_cell(task->board);
            row = cell.r;
            col = cell.c;

            if (cell.found) {
                for (int num = 1; num <= task->board->len; num++) {
                    if (is_valid_placement(task->board, row, col, num)) {
                        Sudoku* new_board = deep_copy_sudoku(task->board);
                        new_board->grid[row * new_board->len + col].value = num;

                        // Update bitmasks in the new board
                        unsigned long long bit = 1ULL << (num - 1);
                        int box = (row / new_board->base) * new_board->base + (col / new_board->base);
                        new_board->row_bits[row] |= bit;
                        new_board->col_bits[col] |= bit;
                        new_board->box_bits[box] |= bit;

                        Task* new_task = malloc(sizeof(Task));
                        new_task->board = new_board;
                        new_task->depth = task->depth + 1;

                        queue_push(work_queue, new_task);
                    }
                }
            }
        } else {
            if (solve_sequential(task->board)) {
                pthread_mutex_lock(&solution_mutex);

                if (!solution_found) {
                    solution_found = 1;
                    solved_sudoku = deep_copy_sudoku(task->board);

                    pthread_mutex_lock(&work_queue->lock);
                    pthread_cond_broadcast(&work_queue->not_empty);
                    pthread_mutex_unlock(&work_queue->lock);
                }

                pthread_mutex_unlock(&solution_mutex);
            }
        }

        if (task->depth > 0) {
            free_sudoku(task->board);
        }

        free(task);
    }

    // Report statistics
    printf("Thread %d: processed %ld tasks, idle cycles: %ld (%.2f%%)\n",
           thread_id, tasks_processed, idle_cycles,
           (double)idle_cycles / (idle_cycles + tasks_processed) * 100);

    return NULL;
}


int solve_sequential(Sudoku* sudoku) {
    pthread_mutex_lock(&solution_mutex);
    int found = solution_found;
    pthread_mutex_unlock(&solution_mutex);

    if (found)
        return 0;

    coord_t pos = first_empty_cell(sudoku);

    if (!pos.found)
        return 1;  // Board is full, solution found!

    for (int num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, pos.r, pos.c, num)) {
            // Try this value
            sudoku->grid[pos.r * sudoku->len + pos.c].value = num;

            // Update bitmasks
            unsigned long long bit = 1ULL << (num - 1);
            int box = (pos.r / sudoku->base) * sudoku->base + (pos.c / sudoku->base);
            sudoku->row_bits[pos.r] |= bit;
            sudoku->col_bits[pos.c] |= bit;
            sudoku->box_bits[box] |= bit;

            if (solve_sequential(sudoku))
                return 1;

            // Backtrack
            sudoku->grid[pos.r * sudoku->len + pos.c].value = 0;

            // Update bitmasks (clear the bits)
            sudoku->row_bits[pos.r] &= ~bit;
            sudoku->col_bits[pos.c] &= ~bit;
            sudoku->box_bits[box] &= ~bit;
        }
    }

    return 0;
}


// Sequential solver function (standard backtracking)
int serial_solve(Sudoku* sudoku) {
    coord_t pos = first_empty_cell(sudoku);

    if (!pos.found)
        return 1;

    for (int num = 1; num <= sudoku->len; num++) {
        if (is_valid_placement(sudoku, pos.r, pos.c, num)) {
            // Place the number
            sudoku->grid[pos.r * sudoku->len + pos.c].value = num;

            // Update bitmasks
            unsigned long long bit = 1ULL << (num - 1);
            int box = (pos.r / sudoku->base) * sudoku->base + (pos.c / sudoku->base);
            sudoku->row_bits[pos.r] |= bit;
            sudoku->col_bits[pos.c] |= bit;
            sudoku->box_bits[box] |= bit;

            if (serial_solve(sudoku))
                return 1;

            // Backtrack: remove the number
            sudoku->grid[pos.r * sudoku->len + pos.c].value = 0;

            // Update bitmasks (clear the bits)
            sudoku->row_bits[pos.r] &= ~bit;
            sudoku->col_bits[pos.c] &= ~bit;
            sudoku->box_bits[box] &= ~bit;
        }
    }

    return 0;
}

coord_t first_empty_cell(Sudoku* sudoku) {
    coord_t pos;
    pos.c = 0;
    pos.r = 0;
    pos.found = 0;
    int len = sudoku->len;

    int r, c;

    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            if (sudoku->grid[r * len + c].value == 0) {
                pos.found = 1;
                pos.r = r;
                pos.c = c;
                return pos;
            }
        }
    }

    return pos;
}

// Check whether num can be placed in cell (r,c)
int is_valid_placement(Sudoku* sudoku, int r, int c, int num) {
    unsigned long long bit = 1ULL << (num - 1);
    int box = (r / sudoku->base) * sudoku->base + (c / sudoku->base);

    // If bit is set in any constraint, placement is invalid
    return !(sudoku->row_bits[r] & bit ||
             sudoku->col_bits[c] & bit ||
             sudoku->box_bits[box] & bit);
}

Sudoku* deep_copy_sudoku(Sudoku* parent) {
    Sudoku* child = malloc(sizeof(Sudoku));

    if (!child) {
        return NULL;
    }

    child->base = parent->base;
    child->len = parent->len;
    int len = child->len;

    // Allocate grid memory
    size_t grid_size = len * len * sizeof(Cell);
    child->grid = malloc(grid_size);

    if (!child->grid) {
        free(child);
        return NULL;
    }

    // Fast copy all cells at once
    memcpy(child->grid, parent->grid, grid_size);

    // Allocate and copy bitmasks
    child->row_bits = malloc(len * sizeof(unsigned long long));
    child->col_bits = malloc(len * sizeof(unsigned long long));
    child->box_bits = malloc(len * sizeof(unsigned long long));

    if (!child->row_bits || !child->col_bits || !child->box_bits) {
        free(child->grid);
        free(child->row_bits);
        free(child->col_bits);
        free(child->box_bits);
        free(child);
        return NULL;
    }

    // Copy bitmasks
    memcpy(child->row_bits, parent->row_bits, len * sizeof(unsigned long long));
    memcpy(child->col_bits, parent->col_bits, len * sizeof(unsigned long long));
    memcpy(child->box_bits, parent->box_bits, len * sizeof(unsigned long long));

    return child;
}
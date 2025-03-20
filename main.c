
#include "functions/display_functions.h"
#include "functions/init_sudoku.h"
#include "functions/parallel.h"
#include "functions/task_hash.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

struct timespec ts = {0, 50000000};
struct timespec start, end;

pthread_mutex_t solution_mutex = PTHREAD_MUTEX_INITIALIZER;
bool solution_found = false;

/*
Future improvements
- Set numbers directly in init_sudoku instead of in another functions
- Parallelize
- Change to more optimal types (eg uint_fast8_t)
- Place memory allocations closer together? Both the grid itself and all
pointers to peers
- Improve init_cell_peers? Mutual pointers?
- Vectorize all for loops
*/

/*
- Select the empty cell with the fewest options
- Backtracking solver
*/

// Shared to enable stealing
WorkDeque *deques;
HashTable *hash_table; // To know which states have been visited
Sudoku *solved_sudoku = NULL;  // This will store the solution

typedef struct coord {
    int r;
    int c;
    int found;
} coord_t;

coord_t find_MRV_cell(Sudoku *sudoku);

coord_t first_empty_cell(Sudoku *sudoku);

int is_valid_placement(Sudoku *sudoku, int r, int c, int num);

int is_valid_board(Sudoku *sudoku);

int solve2(Sudoku *sudoku);

int solve(Sudoku *sudoku, WorkDeque *deque, int depth);

void remove_peer_candidates(Cell *cell, int len);

int total_num_candidates(Sudoku *sudoku);

int fully_solved_board(Sudoku *sudoku);

void *worker_thread(void *arg);
void parallel_sudoku_solver(Sudoku *initial_sudoku, int N_THREADS);
Task *steal_from_other_deques(int N_THREADS, int thread_id);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    printf("Side length:          %d\n", base * base);

    int N_THREADS = 8;

    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    Sudoku *sudoku = init_sudoku(base);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Initialization time: %12.9f seconds\n", elapsed_time);

    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_sudoku_solver(sudoku, N_THREADS);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Solve time:           %.9f seconds\n", elapsed_time);
    printf("Solved correctly? %5d\n", fully_solved_board(solved_sudoku));

    free_sudoku(sudoku);
    return 0;
}


int solve(Sudoku *sudoku, WorkDeque *deque, int depth) {
    // Check if solution already found
    if (solution_found) {
        return 0;
    }

    coord_t pos = first_empty_cell(sudoku);
    int len = sudoku->len;

    if (pos.found == 0) { // No empty cells found, DONE!
        return 1;
    }
    
    int r = pos.r;
    int c = pos.c;
    
    // Create parallel tasks only in the first few levels of recursion
    // to avoid excessive task creation
    if (depth < 3) {
        int valid_count = 0;
        for (int num = 1; num <= len; num++) {
            if (is_valid_placement(sudoku, r, c, num)) {
                valid_count++;
            }
        }
        
        // Only parallelize if we have multiple options
        if (valid_count > 1) {
            int first = 1;
            for (int num = 1; num <= len; num++) {
                if (is_valid_placement(sudoku, r, c, num)) {
                    if (first) {
                        // Handle first valid number ourselves
                        sudoku->grid[r * len + c].value = num;
                        first = 0;
                        
                        if (solve(sudoku, deque, depth + 1)) {
                            return 1;
                        }
                        
                        sudoku->grid[r * len + c].value = 0;
                    } else {
                        // Create new tasks for other valid numbers
                        Task *new_task = create_task(sudoku);
                        new_task->sudoku->grid[r * len + c].value = num;
                        
                        deque_push(deque, new_task);
                    }
                }
            }
            return 0; // We've either processed or delegated all options
        }
    }
    
    // Standard backtracking for deeper recursion levels
    for (int num = 1; num <= len; num++) {
        if (is_valid_placement(sudoku, r, c, num)) {
            sudoku->grid[r * len + c].value = num;
            
            if (solve(sudoku, deque, depth + 1)) {
                return 1;
            }
            
            sudoku->grid[r * len + c].value = 0;
        }
    }
    
    return 0;
}

void parallel_sudoku_solver(Sudoku *initial_sudoku, int N_THREADS) {
    deques = malloc(sizeof(WorkDeque) * N_THREADS);
    pthread_t threads[N_THREADS];

    // Initialize deques
    for (int i = 0; i < N_THREADS; i++) {
        deque_init(&deques[i], i, N_THREADS);
    }

    // Create initial tasks and assign them to thread 0
    // They will get stolen by other threads
    int tasks_assigned = 0;

    Sudoku *temp_sudoku = deep_copy_sudoku(initial_sudoku);

    while (tasks_assigned < N_THREADS) { // Some buffer tasks to start them off
        coord_t pos = first_empty_cell(temp_sudoku);
        if (pos.found == 0) {
            printf("No empty cell found before initializing threads.\n");
            free_sudoku(temp_sudoku);
            free(deques);
            exit(EXIT_FAILURE);
        }

        int r = pos.r;
        int c = pos.c;
        int num;

        for (num = 1; num <= initial_sudoku->len; num++) {
            if (is_valid_placement(initial_sudoku, r, c, num)) {
                // Set cell and send to thread 0 deque
                Task *task = create_task(temp_sudoku);
                task->sudoku->grid[r * initial_sudoku->len + c].value = num;
                temp_sudoku->grid[r * initial_sudoku->len + c].value = num;
                deque_push(&deques[0],
                           task); // Put the state in the first workers queue
                tasks_assigned++;
            }
        }
    }

    free_sudoku(temp_sudoku);
    hash_table = (HashTable *)malloc(sizeof(HashTable));
    if (hash_table == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for hash table\n");
        exit(1);
    }

    initialize_hash_table(hash_table);

    // Launch worker threads
    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &deques[i]);
    }


    // Join threads
    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < N_THREADS; i++) {
        if (deques[i].found_by_thread){
        }
    }
    
    /*
    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *current = hash_table->table[i];
        
        // Traverse the linked list in this bucket
        while (current != NULL) {
            printf("  [%d] Task ID: %u\n", count, current->task_id);
            count++;
            current = current->next;
        }
    }
    */

    for (int i = 0; i < N_THREADS; i++)
        pthread_mutex_destroy(&(deques[i].mutex)); // Destroy the thread mutexes
    free(deques);
    free_hash_table(hash_table);
}

void *worker_thread(void *arg) {
    WorkDeque *deque = (WorkDeque *)arg;

    while (true) {
        // Check if solution found
        pthread_mutex_lock(&solution_mutex);
        if (solution_found) {
            pthread_mutex_unlock(&solution_mutex);
            break;
        }
        pthread_mutex_unlock(&solution_mutex);

        // Take a state from own deque
        Task *task = deque_pop(deque);
        if (!task) {
            task = steal_from_other_deques(deque->N_THREADS, deque->thread_id);
            if (!task) {
                // No tasks available, sleep briefly to reduce CPU usage
                nanosleep(&ts, NULL);
                continue;
            }
        }
     
        // Check if this board state has been explored before
        pthread_mutex_lock(&hash_table->mutex);
        if (task_exists_in_hash_table(hash_table, task->task_id)) {
            pthread_mutex_unlock(&hash_table->mutex);
            free_task(task);
            continue;
        }
        
        // Add to hash table
        insert_task_to_hash_table(hash_table, task);
        pthread_mutex_unlock(&hash_table->mutex);
        
        // Call the modified solve function that can create additional tasks
        if (solve(task->sudoku, deque, 0)) {
            pthread_mutex_lock(&solution_mutex);
            if (!solution_found) {
                solution_found = true;
                deque->found_by_thread = 1;
                solved_sudoku = deep_copy_sudoku(task->sudoku);
            }
            pthread_mutex_unlock(&solution_mutex);
            free_task(task);
            break;
        }


        free_task(task);
    }
    return NULL;
}

Task *steal_from_other_deques(int N_THREADS, int thread_id) {
    for (int i = 0; i < N_THREADS; i++) {
        // Don't steal from self
        int steal_deque_id =
            (thread_id + i) % N_THREADS; // Try stealing from other deques
        if (steal_deque_id != thread_id) {
            Task *task = deque_steal(&deques[steal_deque_id]); // Steal a task
            if (task) {
                return task; // Successfully stole a task
            }
        }
    }
    return NULL; // No task available to steal
}

coord_t first_empty_cell(Sudoku *sudoku) {
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

int is_valid_placement(Sudoku *sudoku, int r, int c, int num) {
    int len = sudoku->len;
    int base = sudoku->base;
    
    // Check row constraints
    int row, col, box_r, box_c;
    for (col = 0; col < len; col++) {
        if (col != c && sudoku->grid[r * len + col].value == num) {
            return 0; // Found same number in row
        }
    }
    
    // Check column constraints
    for (row = 0; row < len; row++) {
        if (row != r && sudoku->grid[row * len + c].value == num) {
            return 0; // Found same number in column
        }
    }
    
    // Check box constraints
    int box_row_start = (r / base) * base;
    int box_col_start = (c / base) * base;

    
    for (box_r = box_row_start; box_r < box_row_start + base; box_r++) {
        for (box_c = box_col_start; box_c < box_col_start + base; box_c++) {
            if ((box_r != r || box_c != c) && sudoku->grid[box_r * len + box_c].value == num) {
                return 0; // Found same number in box
            }
        }
    }
    
    return 1; // Valid placement - no conflicts found
}

int fully_solved_board(Sudoku *sudoku) {
    int len = sudoku->len;
    int r, c;
    for (r = 0; r < sudoku->len; r++) {
        for (c = 0; c < sudoku->len; c++) {
            int val = sudoku->grid[r * len + c].value;
            if (val == 0 || !is_valid_placement(sudoku, r, c, val)) {
                return 0; // Either unfilled or invalid cell
            }
        }
    }
    return 1;
}
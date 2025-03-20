#include "parallel.h"
#include "display_functions.h"
#include "init_sudoku.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

Sudoku *deep_copy_sudoku(Sudoku *parent);

u_int32_t generate_task_id(Sudoku *sudoku);

// Create a new state by deep copying the current state of the board
Task *create_task(Sudoku *parent) {
    Task *new_task = malloc(sizeof(Task));

    new_task->sudoku = deep_copy_sudoku(parent);
    new_task->task_id = generate_task_id(new_task->sudoku);
    return new_task;
}

void free_task(Task *task) {
    free(task->sudoku);
    free(task);
}

// A simple hash function for generating task ID based on Sudoku grid
u_int32_t generate_task_id(Sudoku *sudoku) {
    u_int32_t hash = 0;
    int len = sudoku->len;

    for (int i = 0; i < len * len; i++) {
        hash = hash * 31 + sudoku->grid[i].value;
    }

    return hash;
}

Sudoku *deep_copy_sudoku(Sudoku *parent) {
    Sudoku *child = malloc(sizeof(Sudoku));
    if (!child) {
        printf("Sudoku deep copy failed.\n");
        exit(EXIT_FAILURE);
    }
    
    child->base = parent->base;
    child->len = parent->len;
    int len = child->len;
    int base = child->base;
    
    // Grid memory
    child->grid = malloc(len * len * sizeof(Cell));
    if (!child->grid) {
        free(child);
        printf("Grid deep copy failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // Peer memory
    int r, c;
    int total_peer_count = 3 * (len - 1) * len * len;
    Cell **all_peers = malloc(total_peer_count * sizeof(Cell *));
    if (!all_peers) {
        free(child->grid);
        free(child);
        printf("Peer deep copy failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // First, copy all cell values
    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            int idx = r * len + c;
            child->grid[idx].value = parent->grid[idx].value;
        }
    }
    
    // Then set up all peer relationships
    int peer_offset;
    for (r = 0; r < len; r++) {
        for (c = 0; c < len; c++) {
            int idx = r * len + c;
            peer_offset = 3 * (len - 1) * idx;
            
            // Assign peer pointers
            child->grid[idx].row_peers = all_peers + peer_offset;
            child->grid[idx].col_peers = all_peers + (len - 1) + peer_offset;
            child->grid[idx].box_peers = all_peers + 2 * (len - 1) + peer_offset;
            
            // Set row peers
            int peer_index = 0;
            for (int x = 0; x < len; x++) {
                if (x != c) { // Skip self
                    child->grid[idx].row_peers[peer_index++] = &child->grid[r * len + x];
                }
            }
            
            // Set column peers
            peer_index = 0;
            for (int y = 0; y < len; y++) {
                if (y != r) { // Skip self
                    child->grid[idx].col_peers[peer_index++] = &child->grid[y * len + c];
                }
            }
            
            // Set box peers
            peer_index = 0;
            int box_row_start = (r / base) * base;
            int box_col_start = (c / base) * base;
            
            for (int box_r = box_row_start; box_r < box_row_start + base; box_r++) {
                for (int box_c = box_col_start; box_c < box_col_start + base; box_c++) {
                    if (box_r != r || box_c != c) { // Skip self
                        child->grid[idx].box_peers[peer_index++] = &child->grid[box_r * len + box_c];
                    }
                }
            }
        }
    }
    
    return child;
}

// Initialize deque
void deque_init(WorkDeque *deque, int thread_id, int N_THREADS) {
    deque->top = 0;
    deque->bottom = 0;
    deque->thread_id = thread_id;
    deque->N_THREADS = N_THREADS;
    deque->num_tasks = 0;
    deque->found_by_thread = 0;
    pthread_mutex_init(&deque->mutex, NULL); // Initialize mutex
}

// Push task (LIFO, only owner thread calls this)
void deque_push(WorkDeque *deque, Task *task) {
    pthread_mutex_lock(&deque->mutex); // Lock the mutex

    int b = deque->bottom;
    deque->tasks[b % DEQUE_SIZE] = task;
    deque->bottom = b + 1;
    deque->num_tasks += 1;

    pthread_mutex_unlock(&deque->mutex); // Unlock the mutex
}

Task *deque_pop(WorkDeque *deque) {
    pthread_mutex_lock(&deque->mutex); // Lock the mutex

    int b = deque->bottom - 1; // Remove atomic fetch sub
    int t = deque->top;

    Task *task = NULL;
    if (t <= b) {              // Tasks available
        deque->num_tasks -= 1; // No need for atomic fetch sub
        task = deque->tasks[b % DEQUE_SIZE];
        deque->bottom = b; // Restore bottom
    }

    pthread_mutex_unlock(&deque->mutex); // Unlock the mutex
    return task;
}

Task *deque_steal(WorkDeque *deque) {
    pthread_mutex_lock(&deque->mutex); // Lock the mutex

    int t = deque->top; // Remove atomic load
    int b = deque->bottom;

    Task *task = NULL;
    if (t < b) { // Tasks available
        task = deque->tasks[t % DEQUE_SIZE];

        // Try to steal the task (update top)
        deque->top = t + 1;    // Update top (no atomic CAS)
        deque->num_tasks -= 1; // Decrement num_tasks
    }

    pthread_mutex_unlock(&deque->mutex); // Unlock the mutex
    return task;
}

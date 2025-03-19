#include "parallel.h"
#include "display_functions.h"
#include "init_sudoku.h"
#include <stdio.h>
#include <stdlib.h>

Sudoku *deep_copy_sudoku(Sudoku *parent);
Task *create_task(Sudoku *parent, int row, int col);

Task *create_task(Sudoku *parent, int row, int col) {
    Task *task = malloc(sizeof(Task));

    task->sudoku = deep_copy_sudoku(parent);
    return task;
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

    int peer_offset, peer_index, x, y;
    for (r = 0; r < len; r++) {     // Rows
        for (c = 0; c < len; c++) { // Cols
            int idx = r * len + c;
            peer_offset = 3 * (len - 1) * (idx);

            // Copy value, candidates and number of canidates
            child->grid[idx].value = parent->grid[idx].value;
            // Peer memory slots
            child->grid[idx].row_peers = all_peers + peer_offset;
            child->grid[idx].col_peers = all_peers + (len - 1) + peer_offset;
            child->grid[idx].box_peers =
                all_peers + 2 * (len - 1) + peer_offset;

            peer_index = 0; // Index for row_peers
            for (x = 0; x < c; x++)
                child->grid[r * len + c].row_peers[peer_index++] =
                    &child->grid[r * len + c];

            for (x = c + 1; x < len; x++)
                child->grid[r * len + c].row_peers[peer_index++] =
                    &child->grid[r * len + c];

            // Col peers
            peer_index = 0; // Index for row_peers
            for (y = 0; y < r; y++)
                child->grid[r * len + c].col_peers[peer_index++] =
                    &child->grid[y * len + c];
            for (y = r + 1; y < len; y++)
                child->grid[r * len + c].col_peers[peer_index++] =
                    &child->grid[y * len + c];

            // Box peers
            int br = r / base;
            int bc = c / base;
            peer_index = 0;

            for (x = br * base; x < (br + 1) * base; x++) {
                for (y = bc * base; y < (bc + 1) * base; y++) {
                    if (x == r && y == c) {
                        continue;
                    }
                    child->grid[r * len + c].box_peers[peer_index++] =
                        &child->grid[y * len + c];
                }
            }
        }
    }

    return child;
}


// Initialize deque
void deque_init(WorkDeque *deque) {
    deque->top = 0;
    deque->bottom = 0;
}

// Push task (LIFO, only owner thread calls this)
void deque_push(WorkDeque *deque, Task *task) {
    int b = atomic_load(&deque->bottom);
    deque->tasks[b % DEQUE_SIZE] = task;
    atomic_store(&deque->bottom, b + 1);
}

// Pop task (LIFO, only owner thread calls this)
Task *deque_pop(WorkDeque *deque) {
    int b = atomic_load(&deque->bottom) - 1;
    atomic_store(&deque->bottom, b);
    int t = atomic_load(&deque->top);

    if (t <= b) {  // Still has tasks
        return deque->tasks[b % DEQUE_SIZE];
    } else {  // No more tasks
        atomic_store(&deque->bottom, t);
        return NULL;
    }
}

// Steal task (FIFO, called by other threads)
Task *deque_steal(WorkDeque *deque) {
    int t = atomic_load(&deque->top);
    int b = atomic_load(&deque->bottom);

    if (t < b) { // Tasks available
        Task *task = deque->tasks[t % DEQUE_SIZE];
        atomic_store(&deque->top, t + 1);
        return task;
    } 
    return NULL; // No tasks to steal
}
/*
void worker_thread(WorkDeque *deque, int thread_id) {
    while (true) {
        Task *task = pop_bottom(deque);
        
        if (!task) {
            task = steal_from_other_deques();  // Try to steal a task
            if (!task) continue;  // If stealing fails, keep trying
        }
        
        if (solve_sudoku(task->sudoku, task->row, task->col)) {
            // Sudoku solved! Store result somewhere
        } else {
            // Create new tasks and push them
        for (each valid move) {
            Task *new_task = create_task(task->sudoku, new_row, new_col);
            push_bottom(deque, new_task);
        }
    }
    
    free_task(task); // Clean up memory
}
}

void parallel_sudoku_solver(Sudoku *initial_sudoku) {
    int num_threads = get_num_threads();
    WorkDeque deques[num_threads];
    
    // Initialize deques
    for (int i = 0; i < num_threads; i++) {
        init_deque(&deques[i], INIT_CAPACITY);
    }
    
    // Create initial tasks
    for (each valid move) {
        Task *task = create_task(initial_sudoku, row, col);
        push_bottom(&deques[0], task); // Put them in the first worker's deque
    }
    
    // Launch worker threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &deques[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

*/
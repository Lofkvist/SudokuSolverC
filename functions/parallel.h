#ifndef PARALLEL_H
#define PARALLEL_H
#include "init_sudoku.h"
#include <stdint.h>
#include <stdlib.h>

#define DEQUE_SIZE 1024  // Can be adjusted based on problem size
/*
typedef struct {
    Sudoku* sudoku;
    uint32_t task_id;
} Task;
*/

Sudoku *deep_copy_sudoku(Sudoku *parent);
Task *create_task(Sudoku *parent);
void free_task(Task* task);

typedef struct Deque {
    Task* tasks[DEQUE_SIZE];       // Array of tasks
    int thread_id;
    int N_THREADS;
    int num_tasks;
    int found_by_thread;
    int top;     // Index for work stealing
    int bottom;         // Local worker index
    pthread_mutex_t mutex;  // Mutex for locking the deque
} WorkDeque;


Task *deque_pop(WorkDeque *deque);
Task *deque_steal(WorkDeque *deque);
void deque_push(WorkDeque *deque, Task *task);
void deque_init(WorkDeque *deque, int thread_id, int N_THREADS);

#endif // PARALLEL_H
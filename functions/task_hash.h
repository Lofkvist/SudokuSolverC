
#ifndef TASK_HASH_H
#define TASK_HASH_H
#include <pthread.h>
#include "parallel.h"



#define TABLE_SIZE 10000  // Size of the hash table

typedef struct HashNode {
    u_int32_t task_id;       // Task ID generated from the Sudoku grid
    struct HashNode *next;   // Pointer to next node in case of collision
} HashNode;

typedef struct HashTable {
    HashNode *table[TABLE_SIZE];  // Array of hash nodes
    pthread_mutex_t mutex;        // Mutex for thread-safe access
} HashTable;

// Function declarations

int task_exists_in_hash_table(HashTable* hash_table, u_int32_t task_id);
void insert_task_to_hash_table(HashTable *hash_table, Task *task);
void initialize_hash_table(HashTable* hash_table);
void free_hash_table(HashTable* hash_table);

#endif // TASK_HASH_H
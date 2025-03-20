#include "task_hash.h"
#include <stdio.h>
#include <time.h>
void initialize_hash_table(HashTable *hash_table) {
    if (hash_table == NULL) return;
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        hash_table->table[i] = NULL;
    }
    pthread_mutex_init(&hash_table->mutex, NULL);
}

void insert_task_to_hash_table(HashTable *hash_table, Task *task) {
    if (hash_table == NULL || task == NULL) return;
    
    int index = task->task_id % TABLE_SIZE;
    
    // Create new node with just the task_id
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    if (node == NULL) {
        fprintf(stderr, "Hash node memory allocation failed.\n");
        return;
    }
    
    node->task_id = task->task_id;
    node->next = hash_table->table[index];
    hash_table->table[index] = node;
}

int task_exists_in_hash_table(HashTable* hash_table, u_int32_t task_id) {
    if (hash_table == NULL) return 0;
    
    int index = task_id % TABLE_SIZE;
    
    HashNode *current = hash_table->table[index];
    while (current != NULL) {
        if (current->task_id == task_id) {
            return 1;  // Task exists
        }
        current = current->next;
    }
    
    return 0;  // Task does not exist
}

void free_hash_table(HashTable* hash_table) {
    if (hash_table == NULL) return;
    
    // Free all nodes in each bucket
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *current = hash_table->table[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp);
        }
        hash_table->table[i] = NULL;
    }
    
    // Destroy the mutex
    pthread_mutex_destroy(&(hash_table->mutex));
    
    // Since hash_table was allocated with malloc, we need to free it
    free(hash_table);
}
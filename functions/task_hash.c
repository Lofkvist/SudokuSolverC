#include "task_hash.h"

// Function to initialize the hash table
void initialize_hash_table(HashTable* hash_table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        hash_table->table[i] = NULL;  // Initialize all hash nodes to NULL
    }
    pthread_mutex_init(&hash_table->mutex, NULL);
}

void insert_task_to_hash_table(HashTable* hash_table, Task *task) {
    int index = task->task_id % TABLE_SIZE;  // Simple modulus-based index calculation

    pthread_mutex_lock(&hash_table->mutex);  // Lock the table for thread safety

    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    node->task_id = task->task_id;
    node->task_data = task;
    node->next = hash_table->table[index];  // Insert at the beginning of the list
    hash_table->table[index] = node;

    pthread_mutex_unlock(&hash_table->mutex);  // Unlock the table
}

Task *lookup_task_in_hash_table(HashTable* hash_table, Task *task) {
    int index = task->task_id % TABLE_SIZE;

    pthread_mutex_lock(&hash_table->mutex);  // Lock the table for thread safety

    HashNode *node = hash_table->table[index];
    while (node) {
        if (node->task_id == task->task_id) {
            pthread_mutex_unlock(&hash_table->mutex);  // Unlock the table
            return node->task_data;  // Found the task, return it
        }
        node = node->next;
    }

    pthread_mutex_unlock(&hash_table->mutex);  // Unlock the table
    return NULL;  // Task not found
}

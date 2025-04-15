#include "explored_states.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

ExploredValues* init_explored_values(int len){
    ExploredValues *explored = malloc(sizeof(ExploredValues));
    int total_len = len*len;
    explored->len = len;
    explored->locks = malloc(len*sizeof(pthread_mutex_t));
    explored->tried = calloc(total_len, sizeof(uint_fast64_t));

    for (int i = 0; i < len; i++) {
        pthread_mutex_init(&explored->locks[i], NULL);
    }
    return explored;
}

// Check if a value has been tried
int is_value_tried(ExploredValues *explored, int row, int col, int value) {
    int idx = row * explored->len + col;
    pthread_mutex_lock(&explored->locks[row]);
    int result = (explored->tried[idx] & (1ULL << (value-1))) != 0;
    if (result)
        printf("%d in (%d, %d) has been tried already\n", value, row, col);
    pthread_mutex_unlock(&explored->locks[row]);
    return result;
}

// Mark a value as tried
void mark_value_tried(ExploredValues *explored, int row, int col, int value) {
    int idx = row * explored->len + col;
    pthread_mutex_lock(&explored->locks[row]);
    explored->tried[idx] |= (1ULL << (value-1));
    pthread_mutex_unlock(&explored->locks[row]);
}


// Free resources
void free_explored_values(ExploredValues *explored) {
    for (int i = 0; i < explored->len; i++) {
        pthread_mutex_destroy(&explored->locks[i]);
    }
    free(explored->tried);
    free(explored->locks);
    free(explored);
}
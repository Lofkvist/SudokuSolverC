#include <pthread.h>
#include <stdint.h>

typedef struct {
    uint64_t *tried;         // 1D array of bitmasks
    int len;                // Grid size
    pthread_mutex_t *locks;  // Array of locks for fine-grained locking
} ExploredValues;

void init_explored_values(ExploredValues* explored_states, int len);
int is_value_tried(ExploredValues *explored, int row, int col, int value);
void mark_value_tried(ExploredValues *explored, int row, int col, int value);
void free_explored_values(ExploredValues *explored);
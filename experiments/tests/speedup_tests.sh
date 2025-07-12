#!/bin/bash

# -------- CONFIG --------

# Path to your compiled executable
EXEC="../main"

# Grid directory
GRID_DIR="grids"

# Thread counts to test
THREAD_LIST=(1 2 4 8 16 32)

# Number of times to repeat each test
NUM_RUNS=10

# Output CSV file
CSV_FILE="speedup_results.csv"

# Initialize CSV
echo "BASE,NUM_THREADS,RUN,RUN_TIME" > "$CSV_FILE"

# Define parameter sets for each base
declare -A BASE_DEPTHS
declare -A MIN_TASKS

BASE_DEPTHS[5]=1
MIN_TASKS[5]=2200

BASE_DEPTHS[6]=153
MIN_TASKS[6]=2200

BASE_DEPTHS[8]=171
MIN_TASKS[8]=4400

# -------- RUN TESTS --------

for BASE in 5 6 8; do
    GRID_SIZE=$(( BASE * BASE ))
    GRID_FILE="$GRID_DIR/grid_${GRID_SIZE}x${GRID_SIZE}.dat"
    BASE_DEPTH=${BASE_DEPTHS[$BASE]}
    MIN_TASK_COUNT=${MIN_TASKS[$BASE]}
    
    for NUM_THREADS in "${THREAD_LIST[@]}"; do
        for (( RUN=1; RUN<=NUM_RUNS; RUN++ )); do

            # Run the executable and capture runtime
            # Assuming your program prints only the runtime in seconds when VERBOSE_PERFORMANCE_PRINT is off
            RUNTIME=$($EXEC $BASE $GRID_FILE $NUM_THREADS $BASE_DEPTH $MIN_TASK_COUNT)

            # Append to CSV
            echo "${BASE},${NUM_THREADS},${RUN},${RUNTIME}" >> "$CSV_FILE"

        done
    done
done

echo "All tests completed. Results saved in $CSV_FILE"

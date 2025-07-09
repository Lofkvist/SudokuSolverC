#!/bin/bash

BOARD_BASE=6
NUM_THREADS=8
LOGFILE="results_expdecay5.log"
RUNTIME_LOGFILE="runtimes_expdecay.csv"
NUM_RUNS=6
TIMEOUT_SECONDS=7

echo "" > $LOGFILE # Clear previous log
echo "BASE_DEPTH,MINIMUM_TASK_COUNT,RUN_NUMBER,RUNTIME" > $RUNTIME_LOGFILE # Create CSV header

BASE_DEPTH_START=50
BASE_DEPTH_END=100
BASE_DEPTH_STEP=4

MINIMUM_TASK_COUNT_START=1000
MINIMUM_TASK_COUNT_END=10000
MINIMUM_TASK_COUNT_STEP=200

# Path to the executable in the parent directory
EXEC_PATH="../main" # Assuming 'main' is your executable

# Define the grid filename path
GRID_FILENAME="../grids/grid_$(($BOARD_BASE * $BOARD_BASE))x$(($BOARD_BASE * $BOARD_BASE)).dat"

for BASE_DEPTH in $(seq $BASE_DEPTH_START $BASE_DEPTH_STEP $BASE_DEPTH_END); do
    for MINIMUM_TASK_COUNT in $(seq $MINIMUM_TASK_COUNT_START $MINIMUM_TASK_COUNT_STEP $MINIMUM_TASK_COUNT_END); do
        echo "DEPTH=$BASE_DEPTH TASKS=$MINIMUM_TASK_COUNT" | tee -a $LOGFILE
        
        for ((i = 1; i <= NUM_RUNS; i++)); do
            echo " Run $i" | tee -a $LOGFILE
            
            # Run the program with a timeout
            RUNTIME=$(timeout $TIMEOUT_SECONDS $EXEC_PATH \
                $BOARD_BASE \
                $GRID_FILENAME \
                $NUM_THREADS \
                $BASE_DEPTH \
                $MINIMUM_TASK_COUNT)
            
            # Check if the command timed out
            if [ $? -eq 124 ]; then
                echo "Timeout exceeded ($TIMEOUT_SECONDS seconds)" | tee -a $LOGFILE
                RUNTIME="nan"
            fi
            
            # Write the runtime to the log file
            echo "$RUNTIME" | tee -a $LOGFILE
            
            # Add the runtime to the CSV file
            echo "$BASE_DEPTH,$MINIMUM_TASK_COUNT,$i,$RUNTIME" >> $RUNTIME_LOGFILE
        done
    done
done
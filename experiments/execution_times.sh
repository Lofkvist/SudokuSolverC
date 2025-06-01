#!/bin/bash

BOARD_BASE=8
NUM_THREADS=8
LOGFILE="result_base8_74depth.log"
RUNTIME_LOGFILE="result_base8.csv"
NUM_RUNS=10
TIMEOUT_SECONDS=30

echo "" > $LOGFILE # Clear previous log
echo "BASE_DEPTH,MINIMUM_TASK_COUNT,RUN_NUMBER,RUNTIME" > $RUNTIME_LOGFILE # Create CSV header

DEPTH_START=50
DEPTH_END=300
DEPTH_STEP=20
DEPTH=74


MIN_TASK_COUNT_START=10000
MIN_TASK_COUNT_END=50000
MIN_TASK_COUNT_STEP=2000

EXEC_PATH="../main" # Path to your executable
GRID_FILENAME="../grids/grid_$(($BOARD_BASE * $BOARD_BASE))x$(($BOARD_BASE * $BOARD_BASE)).dat"

    for MINIMUM_TASK_COUNT in $(seq $MIN_TASK_COUNT_START $MIN_TASK_COUNT_STEP $MIN_TASK_COUNT_END); do
        echo "DEPTH=$DEPTH TASKS=$MINIMUM_TASK_COUNT" | tee -a $LOGFILE

        for ((i = 1; i <= NUM_RUNS; i++)); do
            echo " Run $i" | tee -a $LOGFILE

            RUNTIME=$(timeout $TIMEOUT_SECONDS $EXEC_PATH \
                $BOARD_BASE \
                $GRID_FILENAME \
                $NUM_THREADS \
                $DEPTH \
                $MINIMUM_TASK_COUNT)

            if [ $? -eq 124 ]; then
                echo "Timeout exceeded ($TIMEOUT_SECONDS seconds)" | tee -a $LOGFILE
                RUNTIME="nan"
            fi

            echo "$RUNTIME" | tee -a $LOGFILE
            echo "$BASE_DEPTH,$MINIMUM_TASK_COUNT,$i,$RUNTIME" >> $RUNTIME_LOGFILE
        done
    done

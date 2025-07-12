#!/bin/bash -l

# --------------------------------------------------------
# Configurable parameters
# --------------------------------------------------------

BOARD_BASE=8

RUNTIME_LOGFILE="vitsippa_base${BOARD_BASE}.csv"
NUM_RUNS=1
TIMEOUT_SECONDS=60

DEPTH_START=75
DEPTH_END=200
DEPTH_STEP=6

MIN_TASK_COUNT_START=2000
MIN_TASK_COUNT_END=5000
MIN_TASK_COUNT_STEP=400

EXEC_PATH="../main"
GRID_FILENAME="../grids/grid_$(($BOARD_BASE * $BOARD_BASE))x$(($BOARD_BASE * $BOARD_BASE)).dat"

THREAD_LIST=(1 2 4 8 16 32)

# --------------------------------------------------------
# Execution
# --------------------------------------------------------

# Create CSV header if file does not exist
if [ ! -f "$RUNTIME_LOGFILE" ]; then
    echo "NUM_THREADS,BASE_DEPTH,MINIMUM_TASK_COUNT,RUN_NUMBER,RUNTIME" > "$RUNTIME_LOGFILE"
fi

for NUM_THREADS in "${THREAD_LIST[@]}"; do
    for BASE_DEPTH in $(seq $DEPTH_START $DEPTH_STEP $DEPTH_END); do
        for MINIMUM_TASK_COUNT in $(seq $MIN_TASK_COUNT_START $MIN_TASK_COUNT_STEP $MIN_TASK_COUNT_END); do
            for ((i = 1; i <= NUM_RUNS; i++)); do

                # Check if this combination is already logged
                grep -q -E "^$NUM_THREADS,$BASE_DEPTH,$MINIMUM_TASK_COUNT,$i," "$RUNTIME_LOGFILE"
                if [ $? -eq 0 ]; then
                    continue
                fi

                RUNTIME=$(timeout $TIMEOUT_SECONDS $EXEC_PATH \
                    $BOARD_BASE \
                    $GRID_FILENAME \
                    $NUM_THREADS \
                    $BASE_DEPTH \
                    $MINIMUM_TASK_COUNT)

                if [ $? -eq 124 ]; then
                    RUNTIME="nan"
                fi

                echo "$NUM_THREADS,$BASE_DEPTH,$MINIMUM_TASK_COUNT,$i,$RUNTIME" >> "$RUNTIME_LOGFILE"

            done
        done
    done
done

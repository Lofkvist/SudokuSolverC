#!/bin/bash -l

#SBATCH -A uppmax2025-2-247
#SBATCH -p node
#SBATCH -N 2
#SBATCH -t 30:00:00
#SBATCH -J sudoku_gridsearch
#SBATCH -e error.log

module load gcc
make clean -C ..
make -C ..
# --------------------------------------------------------
# Configurable parameters
# --------------------------------------------------------

# Either pass BOARD_BASE as an argument to the script, or set a default
BOARD_BASE=6      # Default to 5 if not provided

RUNTIME_LOGFILE="result_base${BOARD_BASE}.csv"
NUM_RUNS=1
TIMEOUT_SECONDS=10 # Quit executing after 1 min

DEPTH_START=2
DEPTH_END=200
DEPTH_STEP=5

MIN_TASK_COUNT_START=2
MIN_TASK_COUNT_END=5000
MIN_TASK_COUNT_STEP=50

EXEC_PATH="../main" # Path to your executable
GRID_FILENAME="../grids/grid_$(($BOARD_BASE * $BOARD_BASE))x$(($BOARD_BASE * $BOARD_BASE)).dat"

THREAD_LIST=(1 2 4 8 16 32)

# --------------------------------------------------------
# Execution
# --------------------------------------------------------

# Create CSV header
echo "NUM_THREADS,BASE_DEPTH,MINIMUM_TASK_COUNT,RUN_NUMBER,RUNTIME" > $RUNTIME_LOGFILE

for NUM_THREADS in "${THREAD_LIST[@]}"; do
    for BASE_DEPTH in $(seq $DEPTH_START $DEPTH_STEP $DEPTH_END); do
        for MINIMUM_TASK_COUNT in $(seq $MIN_TASK_COUNT_START $MIN_TASK_COUNT_STEP $MIN_TASK_COUNT_END); do

            for ((i = 1; i <= NUM_RUNS; i++)); do

                echo "Running: Threads=$NUM_THREADS, Depth=$BASE_DEPTH, Tasks=$MINIMUM_TASK_COUNT, Run=$i"

                # Run executable with timeout
                RUNTIME=$(timeout $TIMEOUT_SECONDS $EXEC_PATH \
                    $BOARD_BASE \
                    $GRID_FILENAME \
                    $NUM_THREADS \
                    $BASE_DEPTH \
                    $MINIMUM_TASK_COUNT)

                if [ $? -eq 124 ]; then
                    RUNTIME="nan"
                    echo "Timeout for Threads=$NUM_THREADS Depth=$BASE_DEPTH Tasks=$MINIMUM_TASK_COUNT Run=$i"
                fi

                # Append result to CSV
                echo "$NUM_THREADS,$BASE_DEPTH,$MINIMUM_TASK_COUNT,$i,$RUNTIME" >> $RUNTIME_LOGFILE

            done
        done
    done
done

# These options apply when running ´make run´
BOARD_BASE := 6  # Options: 5, 6, 8
NUM_THREADS := 8
DEPTH_SWITCHING_POINT := 70
MINIMUM_TASK_COUNT := 500
BATCH_SIZE = 200

CC := gcc
CFLAGS := -O3 -Wall -lpthread -fopenmp -march=native -ffast-math
TARGET := main
SRC_FILES := main.c source_files/*.c

# Default target
all: $(TARGET)

# Compile the main program
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Run the program with configured board base
run: $(TARGET)
	./$(TARGET) $(BOARD_BASE) $(NUM_THREADS) $(DEPTH_SWITCHING_POINT) $(MINIMUM_TASK_COUNT) $(BATCH_SIZE)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all run clean

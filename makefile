# These options apply when running ´make run´
BOARD_BASE := 8 # Options: 5, 6, 8
FILENAME := "grids/grid_$(shell echo $$(( $(BOARD_BASE) * $(BOARD_BASE) )))x$(shell echo $$(( $(BOARD_BASE) * $(BOARD_BASE) ))).dat"
NUM_THREADS := 16
BASE_DEPTH := 105
MINIMUM_TASK_COUNT := 2000

CC := gcc
CFLAGS := -O3 -Wall -march=native -ffast-math -fopenmp -fno-stack-protector
LIBS := -lm
TARGET := main
SRC_FILES := main.c source_files/*.c

# Default target
all: $(TARGET)

# Compile the main program
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Run the program with configured board base
run: $(TARGET)
	./$(TARGET) $(BOARD_BASE) $(FILENAME) $(NUM_THREADS) $(BASE_DEPTH) $(MINIMUM_TASK_COUNT)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all run clean

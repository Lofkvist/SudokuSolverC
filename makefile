# Configuration
BOARD_BASE := 6  # Options: 5, 6, 8
NUM_THREADS := 8
DEPTH_LIMIT := 12
CC := gcc
CFLAGS := -O3 -Wall -lpthread -fopenmp
TARGET := main
SRC_FILES := main.c functions/display_functions.c functions/init_sudoku.c functions/explored_states.c

# Default target
all: $(TARGET)

# Compile the main program
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Run the program with configured board base
run: $(TARGET)
	./$(TARGET) $(BOARD_BASE) $(NUM_THREADS) $(DEPTH_LIMIT)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all run clean
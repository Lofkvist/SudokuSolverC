# Configuration
BOARD_BASE := 6  # Options: 5, 6, 8
CC := gcc
CFLAGS := -O3 -pg -Wall -lpthread -fopenmp
TARGET := main
SRC_FILES := main.c functions/init_sudoku.c functions/display_functions.c functions/parallel.c functions/task_hash.c

# Default target
all: $(TARGET)

# Compile the main program
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Run the program with configured board base
run: $(TARGET)
	./$(TARGET) $(BOARD_BASE)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all run clean
# Configuration
BOARD_BASE := 3  # Options: 3, 4, 5, 6, 7, 8
CC := gcc
CFLAGS := -O3 -g -Wall
TARGET := main
SRC_FILES := main.c init_sudoku.c cell_bit_operations.c

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
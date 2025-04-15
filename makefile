# Configuration
BOARD_BASE := 5  # Options: 5, 6, 8
CC := gcc
CFLAGS := -O3 -Wall -lpthread -fopenmp
TARGET := main
SRC_FILES := main.c functions/*.c

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
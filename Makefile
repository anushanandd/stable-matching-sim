CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Iinclude
LDFLAGS = -lm

# Source files
SOURCES = src/main.c src/matching.c src/verification.c src/existence.c src/generators.c src/benchmark.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = k_stable_matching

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run tests
test: $(TARGET)
	./$(TARGET) --test

# Run benchmarks
benchmark: $(TARGET)
	./$(TARGET) --benchmark

# Run comprehensive tests
test_algorithms: tests/test_algorithms.c $(OBJECTS)
	$(CC) $(CFLAGS) tests/test_algorithms.c $(filter-out src/main.o, $(OBJECTS)) -o tests/test_algorithms $(LDFLAGS)
	./tests/test_algorithms

# Create directories
setup:
	mkdir -p src include tests data results

.PHONY: all clean test benchmark setup test_algorithms

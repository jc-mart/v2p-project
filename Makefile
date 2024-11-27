# Compiler and flag options
CC = gcc
CFLAGS = -Iinclude -Itests -Wall -Wextra -std=gnu17
LDFLAGS =

# Source code
SRC = src/rtt_functions/rtt_helper.c
TEST_SRC = tests/test_rtt_helper.c tests/unity.c

# Generated object files
OBJ = $(SRC:src/%.c=build/%.o)
TEST_OBJ = $(TEST_SRC:tests/%.c=build/%.o)

# Targets
TARGET = build/my_project
TEST_TARGET = build/test_my_project

# Default target
all: $(TARGET)

# Build the main project
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Build the test executable
$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files
build/%.o: src/%.c | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: tests/%.c | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Create build directory
build:
	mkdir -p build

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean build directory
clean:
	rm -rf build

.PHONY: cppcheck
cppcheck:
	cppcheck --enable=all --inconclusive --std=c17 --suppress=missingIncludeSystem -Iinclude src/

.PHONY: clang-tidy
clang-tidy:
	clang-tidy src/*.c src/*/*.c -- -Iinclude -std=gnu17

.PHONY: all test clean

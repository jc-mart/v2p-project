# Compiler and flag options
CC = gcc
CFLAGS = -Iinclude -Itests -Wall -Wextra -g -pg
LDFLAGS = -pg

# Source code
SRC = src/rtt_functions/rtt_helper.c src/file_handling/file_handler.c src/networking/rtt_networking.c
TEST_SRC = $(wildcard tests/test_*.c)
UNITY_SRC = tests/unity.c

# Generated object files
OBJ = $(SRC:src/%.c=build/%.o)
TEST_OBJ = $(TEST_SRC:tests/%.c=build/%.o)
UNITY_OBJ = $(UNITY_SRC:tests/%.c=build/%.o)

# Test executables
TEST_EXECUTABLES = $(TEST_SRC:tests/%.c=build/%)

# Targets
TARGET = build/my_project

# Default target
all: $(TARGET)

# Build the main project
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Build the test executables
$(TEST_EXECUTABLES): build/%: build/%.o $(UNITY_OBJ) $(OBJ)
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
test: $(TEST_EXECUTABLES)
	@for test in $(TEST_EXECUTABLES); do ./$$test || exit 1; done

# Clean build directory
clean:
	rm -rf build

# Run Cppcheck
.PHONY: cppcheck
cppcheck:
	cppcheck --enable=all --inconclusive --std=c11 --suppress=missingIncludeSystem -Iinclude src

# Run Clang-Tidy
.PHONY: clang-tidy
clang-tidy:
	clang-tidy -p build src/**/*.c -- -Iinclude

.PHONY: all test clean

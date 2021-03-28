CC=gcc
CFLAGS=-g -Wall
BUILD_DIR=build
TEST_DIR=test_files

all: main

main: main.c static_library
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) $(BUILD_DIR)/main.o $(BUILD_DIR)/library.a -o main

static_library: library.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c library.c -o $(BUILD_DIR)/library.o
	ar rcs $(BUILD_DIR)/library.a $(BUILD_DIR)/library.o 

test: main
	@./main $(shell cat $(TEST_DIR)/test_args.txt) > raport2.txt

clean:
	rm -rf $(BUILD_DIR) *.o *.so *.a main

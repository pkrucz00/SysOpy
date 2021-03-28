CC=gcc
CFLAGS=-g -Wall
BUILD_DIR=build
TEST_DIR=test_files/args

all: main

main: code2.c static_library
	$(CC) $(CFLAGS) -c code2.c -o $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) $(BUILD_DIR)/main.o $(BUILD_DIR)/library.a -o main

static_library: library.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c library.c -o $(BUILD_DIR)/library.o
	ar rcs $(BUILD_DIR)/library.a $(BUILD_DIR)/library.o 

test: main
	printf "With forks\n" >> raport2.txt
	printf "%35s:\t%20s\t%20s\t%20s\n" "name" "real_time [s]" "user_time [s]" "sys_time [s]" >> raport2.txt
	@./main $(shell cat $(TEST_DIR)/test_args1.txt) >> raport2.txt
	@./main $(shell cat $(TEST_DIR)/test_args2.txt) >> raport2.txt
	@./main $(shell cat $(TEST_DIR)/test_args3.txt) >> raport2.txt
	@./main $(shell cat $(TEST_DIR)/test_args4.txt) >> raport2.txt

clean:
	rm -rf main $(BUILD_DIR) *.o *.so *.a 

CC=gcc
CFLAGS=-g -Wall
STATIC_DIR=static_library_dir
SHARED_DIR=shared_library_dir
TEST_DIR=test_files
O_type=O1

all: test

static_library: library.c
	mkdir -p $(STATIC_DIR)
	$(CC) -c  $(CFLAGS) library.c -o $(STATIC_DIR)/library.o
	ar rcs $(STATIC_DIR)/library.a $(STATIC_DIR)/library.o 

shared_library: library.c
	mkdir -p $(SHARED_DIR)
	$(CC) -c $(CFLAGS) -fPIC library.c -o $(SHARED_DIR)/library.o
	$(CC) -shared -fPIC -o $(SHARED_DIR)/liblibrary.so $(SHARED_DIR)/library.o

main_with_static: main.c static_library
	$(CC) $(CFLAGS) -c main.c -o $(STATIC_DIR)/main.o
	$(CC) $(CFLAGS) -$(O_type) $(STATIC_DIR)/main.o $(STATIC_DIR)/library.a -o main

main_with_shared: main.c shared_library
	$(CC) $(CFLAGS) main.c -$(O_type) -L./$(SHARED_DIR) -Wl,-rpath=./$(SHARED_DIR) -llibrary -o main

main_with_dynamic: main.c shared_library
	$(CC) $(CFLAGS) main.c -$(O_type) -ldl -o main -D DLL

test:
	printf "TESTED WITH STATIC\n" > results3b.txt
	printf "O0:\n" >> results3b.txt
	make main_with_static O_type=O0 
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean

	printf "O1:\n" >> results3b.txt
	make main_with_static O_type=O1
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean

	printf "O2:\n" >> results3b.txt
	make main_with_static O_type=O2
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean
	
	printf "O3:\n" >> results3b.txt
	make main_with_static O_type=O3
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean
	
	printf "Ofast:\n" >> results3b.txt
	make main_with_static O_type=Ofast
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean
	
	printf "Os:\n" >> results3b.txt
	make main_with_static O_type=Os
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3b.txt
	make clean

clean:
	rm -rf main $(STATIC_DIR) $(SHARED_DIR) *.o *.so *.a 

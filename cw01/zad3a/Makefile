CC=gcc
CFLAGS=-g -Wall
STATIC_DIR=static_library_dir
SHARED_DIR=shared_library_dir
TEST_DIR=test_files

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
	$(CC) $(CFLAGS) $(STATIC_DIR)/main.o $(STATIC_DIR)/library.a -o main

main_with_shared: main.c shared_library
	$(CC) $(CFLAGS) main.c -L./$(SHARED_DIR) -Wl,-rpath=./$(SHARED_DIR) -llibrary -o main

main_with_dynamic: main.c shared_library
	$(CC) $(CFLAGS) main.c -ldl -o main -D DLL

test:
	printf "main_with_static:\n" > results3a.txt
	make main_with_static
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3a.txt
	make clean

	printf "\nmain_with_shared:\n" >> results3a.txt
	make main_with_shared
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3a.txt
	make clean

	printf "\nmain_with_dynamic:\n" >> results3a.txt
	make main_with_dynamic
	@./main $(shell cat $(TEST_DIR)/test_args.txt) >> results3a.txt
	make clean

clean:
	rm -rf main $(STATIC_DIR) $(SHARED_DIR) *.o *.so *.a 

CC=gcc
CFLAGS=-g -c -Wall
STATIC_DIR=static_library_dir
SHARED_DIR=shared_library_dir

all: static_library shared_library

static_library: library.c
	mkdir -p $(STATIC_DIR)
	$(CC) $(CFLAGS) library.c -o $(STATIC_DIR)/library.o
	ar rcs $(STATIC_DIR)/library.a $(STATIC_DIR)/library.o 

shared_library: library.c
	mkdir -p $(SHARED_DIR)
	$(CC) $(CFLAGS) -fPIC library.c -o $(SHARED_DIR)/library.o
	$(CC) -shared -fPIC -o $(SHARED_DIR)/libdiff.so $(SHARED_DIR)/library.o


clean:
	rm -rf $(STATIC_DIR) $(SHARED_DIR) *.o *.so *.a 

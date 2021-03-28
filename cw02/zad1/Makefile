CC=gcc
CFLAGS=-g -Wall

all: main

main: code1_sys.c
	$(CC) $(CFLAGS) -c code1.c -o main.o
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm *.o main
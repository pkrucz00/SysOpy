CC=gcc
CFLAGS=-g -Wall

all: main

main: code4.c
	$(CC) $(CFLAGS) -c code4.c -o main.o
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm main *.o
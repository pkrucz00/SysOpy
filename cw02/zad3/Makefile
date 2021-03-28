CC=gcc
CFLAGS=-g -Wall

all: main

main: code3.c
	$(CC) $(CFLAGS) -c code3.c -o main.o
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm main *.o
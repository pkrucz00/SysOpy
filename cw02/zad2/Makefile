CC=gcc
CFLAGS=-g -Wall

all: main

main: code2.c
	$(CC) $(CFLAGS) -c code2.c -o main.o
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm main *.o
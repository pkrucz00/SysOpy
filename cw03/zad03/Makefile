CC=gcc
CFLAGS=-g -Wall

all: main

main: code3.c
	$(CC) -c $(CFLAGS) code3.c
	$(CC) $(CFLAGS) code3.o -o main

clean:
	rm main *.o
CC=gcc
CFLAGS=-g -Wall

all: main

main: code1.c
	$(CC) -c $(CFLAGS) code1.c
	$(CC) $(CFLAGS) code1.o -o main

clean:
	rm main *.o
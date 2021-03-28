CC=gcc
CFLAGS=-g -Wall

all: main

main: code5.c
	$(CC) $(CFLAGS) -c code5.c -o main.o
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm main *.o
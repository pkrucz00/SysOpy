CC=gcc
CFLAGS=-Wall -g

all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main -lm -pthread

test: main
	./main 1 numbers saturn.ascii.pgm test.pgm > Times.txt
	./main 2 numbers saturn.ascii.pgm test.pgm >> Times.txt
	./main 4 numbers saturn.ascii.pgm test.pgm >> Times.txt
	./main 8 numbers saturn.ascii.pgm test.pgm >> Times.txt
	./main 16 numbers saturn.ascii.pgm test.pgm >> Times.txt

	./main 1  block saturn.ascii.pgm test.pgm >> Times.txt
	./main 2  block saturn.ascii.pgm test.pgm >> Times.txt
	./main 4  block saturn.ascii.pgm test.pgm >> Times.txt
	./main 8  block saturn.ascii.pgm test.pgm >> Times.txt
	./main 16 block saturn.ascii.pgm test.pgm >> Times.txt

clean: 
	rm -rf main
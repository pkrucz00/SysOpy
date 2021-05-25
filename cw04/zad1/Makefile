CC=gcc -Wall -g

all: main

main: main.c exec.c
	$(CC) exec.c -o exec
	$(CC) main.c -o main
	

clean: 
	rm -rf main exec
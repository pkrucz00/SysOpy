CC=gcc -Wall -g

all: main

main: main.c cook deliverer
	$(CC) common.c main.c -o main -lrt -pthread

cook: cook.c 
	$(CC) common.c cook.c -o cook -lrt -pthread

deliverer: deliverer.c
	$(CC) common.c deliverer.c -o deliverer -lrt -pthread

clean:
	rm -rf main cook deliverer
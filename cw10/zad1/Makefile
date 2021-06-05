CC=gcc -Wall

all: build

build: client.c server.c
	$(CC) -o client client.c common.c -lpthread
	$(CC) -o server server.c common.c -lpthread

clean:
	rm -f client server sock
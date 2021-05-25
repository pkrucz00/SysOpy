CC=gcc
CFLAGS=-g -Wall

all: server client

server: server.c 
	$(CC) $(CFLAGS) server.c -o server -lrt

client: client.c 
	$(CC) $(CFLAGS) client.c -o client -lrt


clean:
	rm server client

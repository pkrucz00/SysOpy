CC=gcc -Wall -g

all: sender catcher

sender: sender.c
	$(CC) sender.c -o sender

catcher: catcher.c
	$(CC) catcher.c -o catcher

clean:
	rm -rf sender catcher
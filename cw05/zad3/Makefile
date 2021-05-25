CC=gcc -Wall -g

all: test clean

test: producer consumer test.c
	$(CC) test.c -o test
	rm -f pipe
	valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./test

producer: producer.c
	$(CC) producer.c -o producer 

consumer: consumer.c
	$(CC) consumer.c vector.c -o consumer

clean: 
	rm -rf producer consumer test input* output 
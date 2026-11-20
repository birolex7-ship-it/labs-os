all: batcher

batcher: main.c sort.c sort.h
	gcc -O2 main.c sort.c -o batcher -pthread

clean:
	rm -f batcher
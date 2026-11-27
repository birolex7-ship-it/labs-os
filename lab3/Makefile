all: parent child

parent: parent.c
	gcc parent.c -o parent -lrt -pthread

child: child.c
	gcc child.c -o child -lrt -pthread

clean:
	rm -f parent child result.txt
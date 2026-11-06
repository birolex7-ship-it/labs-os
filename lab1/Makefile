all: parent child

parent: parent.c
	gcc parent.c -o parent

child: child.c
	gcc child.c -o child

clean:
	rm -f parent child test.txt
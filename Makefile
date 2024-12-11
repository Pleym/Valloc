CC = gcc
CFLAGS = -Wall -Wextra -g

all: test

test: test.o valloc.o
	$(CC) $(CFLAGS) -o test test.o valloc.o

test.o: test.c valloc.h
	$(CC) $(CFLAGS) -c test.c

valloc.o: valloc.c valloc.h
	$(CC) $(CFLAGS) -c valloc.c

clean:
	rm -f test *.o

.PHONY: all clean

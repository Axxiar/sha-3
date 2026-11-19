CC = cc
CFLAGS = -O2 -Wall -Wextra -pedantic -std=c99

all: test_keccak

test_keccak: test_keccak.c keccak.c keccak.h
	$(CC) $(CFLAGS) -o $@ test_keccak.c keccak.c

test: test_keccak
	./test_keccak

clean:
	$(RM) -f test_keccak *.o

.PHONY: all test clean


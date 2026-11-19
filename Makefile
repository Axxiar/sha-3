CC = cc
CFLAGS = -O2 -Wall -Wextra -pedantic -std=c99 -Iinclude

all: test_keccak

test_keccak: test/test_keccak.c src/keccak.c include/keccak.h
	$(CC) $(CFLAGS) -o $@ test/test_keccak.c src/keccak.c

test: test_keccak
	./test_keccak

# Computes constans RC and RHO 
tools/compute_constants: tools/compute_constants.c
	$(CC) $(CFLAGS) -o $@ tools/compute_constants.c

compute-constants: tools/compute_constants
	./tools/compute_constants

clean:
	rm -f test_keccak tools/compute_constants

.PHONY: all test compute-constants clean

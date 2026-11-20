CC = cc
CFLAGS = -O2 -Wall -Wextra -pedantic -std=c99 -Iinclude

all: sha3sum

sha3sum: src/main.c src/sha3.c src/keccak.c include/sha3.h include/keccak.h
	$(CC) $(CFLAGS) -o $@ src/main.c src/sha3.c src/keccak.c

test_keccak: test/test_keccak.c src/keccak.c include/keccak.h
	$(CC) $(CFLAGS) -o $@ test/test_keccak.c src/keccak.c

test_sha3: test/test_sha3.c src/sha3.c src/keccak.c include/sha3.h include/keccak.h
	$(CC) $(CFLAGS) -o $@ test/test_sha3.c src/sha3.c src/keccak.c

test: test_keccak test_sha3
	./test_keccak
	./test_sha3

# Computes constans RC and RHO 
tools/compute_constants: tools/compute_constants.c
	$(CC) $(CFLAGS) -o $@ tools/compute_constants.c

compute-constants: tools/compute_constants
	./tools/compute_constants

clean:
	rm -f test_keccak test_sha3 tools/compute_constants sha3sum

.PHONY: all test compute-constants clean

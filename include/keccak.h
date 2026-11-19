#ifndef KECCAK_H
#define KECCAK_H

#include <stdint.h>

#define KECCAK_LANES 25 // 5 x 5 lanes of 64 bits = 1600 bits
#define KECCAK_ROUNDS 24

/* Index of a lane A[x,y] in an array : x + 5*y */
#define KECCAK_IDX(x, y) ((x) + 5 * (y))

void keccak_f1600(uint64_t A[KECCAK_LANES]);

#endif /* !KECCAK_H */

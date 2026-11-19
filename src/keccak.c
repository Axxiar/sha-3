#include "keccak.h"

#include <stdint.h>

/* Generated from FIPS 202, Algorithms 2 and 5.
 *
 * The program that computed the two following constants is in
 * ../tools/compute_constants.c */
static const uint64_t RC[KECCAK_ROUNDS] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008,
};

static const unsigned int RHO[KECCAK_LANES] = {
    0,  1,  62, 28, 27, // y = 0 : (x = 0..4)
    36, 44, 6,  55, 20, // y = 1 : (x = 0..4)
    3,  10, 43, 25, 39, // y = 2 : (x = 0..4)
    41, 45, 15, 21, 8, // y = 3 : (x = 0..4)
    18, 2,  61, 56, 14, // y = 4 : (x = 0..4)
};

static inline uint64_t rol64(uint64_t v, unsigned int n)
{
    if (n == 0)
        return v;
    return (v << n) | (v >> (64 - n));
}

/* θ step : each bit is XORed with the parity of two neighbouring columns
 * - C[x] is the parity of column x
 * - D[x] combines column x-1 with column x+1, rotated by one bit, so each bit
 * sees the parity of its column and of the neighbouring ones */
static void theta(uint64_t A[KECCAK_LANES])
{
    uint64_t C[5], D[5];
    int x, y;

    for (x = 0; x < 5; x++)
        C[x] = A[KECCAK_IDX(x, 0)] ^ A[KECCAK_IDX(x, 1)] ^ A[KECCAK_IDX(x, 2)]
            ^ A[KECCAK_IDX(x, 3)] ^ A[KECCAK_IDX(x, 4)];

    for (x = 0; x < 5; x++)
        D[x] = C[(x + 4) % 5] ^ rol64(C[(x + 1) % 5], 1);

    // Apply D to all 5 lanes of each column
    for (y = 0; y < 5; y++)
        for (x = 0; x < 5; x++)
            A[KECCAK_IDX(x, y)] ^= D[x];
}

/* ρ step : rotate the bits of each lane by a fixed offset RHO[x,y] */
static void rho(uint64_t A[KECCAK_LANES])
{
    int x, y;

    for (y = 0; y < 5; y++)
        for (x = 0; x < 5; x++)
            A[KECCAK_IDX(x, y)] =
                rol64(A[KECCAK_IDX(x, y)], RHO[KECCAK_IDX(x, y)]);
}

/* π step : rearrange the positions of the lanes */
static void pi(uint64_t A[KECCAK_LANES])
{
    uint64_t B[KECCAK_LANES];
    int x, y;

    // Temporary state B so we read first then write
    for (x = 0; x < 5; x++)
        for (y = 0; y < 5; y++)
            B[KECCAK_IDX(x, y)] = A[KECCAK_IDX((x + 3 * y) % 5, x)];

    for (y = 0; y < 5; y++)
        for (x = 0; x < 5; x++)
            A[KECCAK_IDX(x, y)] = B[KECCAK_IDX(x, y)];
}

/* χ step : XOR each bit with a non-linear function of two other bits in its row
 *
 * A[x] <- A[x] XOR (NOT A[x+1] AND A[x+2]) */
static void chi(uint64_t A[KECCAK_LANES])
{
    uint64_t row[5];
    int x, y;

    for (y = 0; y < 5; y++)
    {
        // Same here, we ensure read first then write
        for (x = 0; x < 5; x++)
            row[x] = A[KECCAK_IDX(x, y)];

        for (x = 0; x < 5; x++)
            A[KECCAK_IDX(x, y)] =
                row[x] ^ ((~row[(x + 1) % 5]) & row[(x + 2) % 5]);
    }
}

/* ι step : modify some bits of lane [0,0] depending on the round index.
 *
 * The other 24 lanes are not affected by ι */
static void iota(uint64_t A[KECCAK_LANES], int round)
{
    A[KECCAK_IDX(0, 0)] ^= RC[round];
}

/* Permutation function Keccak-f[1600] (also Keccak-p[1600, 24]) */
void keccak_f1600(uint64_t A[KECCAK_LANES])
{
    int round;

    for (round = 0; round < KECCAK_ROUNDS; round++)
    {
        theta(A);
        rho(A);
        pi(A);
        chi(A);
        iota(A, round);
    }
}

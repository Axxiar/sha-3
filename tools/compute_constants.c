/* This program computes constants RC (for step ι) and offsets of step ρ for
 * Keccak-f[1600] (following FIPS 202.)
 *
 * It is a standalone program, the output goes to stdout and should be copied
 * into src/keccak.c
 * Usage: make compute_constants */

#include <stdint.h>
#include <stdio.h>

#define KECCAK_ROUNDS 24
#define KECCAK_L 6 /* l = 6 for w = 64 */

/* Algorithm 5: rc(t)
 * R holds 8 bits, R[0] being the first one. R[8] is a temporary bit used
 * during the shift. */
static int rc(int t)
{
    uint8_t R[9];
    int i, k;

    if (t % 255 == 0)
        return 1;

    for (k = 0; k < 9; k++)
        R[k] = 0;
    R[0] = 1; /* R = 10000000 */

    for (i = 0; i < t % 255; i++)
    {
        // R = 0 || R: shift right, a 0 enters on the left
        for (k = 8; k > 0; k--)
            R[k] = R[k - 1];
        R[0] = 0;

        // XOR R[8] into positions 0, 4, 5 and 6
        R[0] ^= R[8];
        R[4] ^= R[8];
        R[5] ^= R[8];
        R[6] ^= R[8];
        // Trunc8: only R[0..7] are kept, R[8] is overwritten by the next shift
    }
    return R[0];
}

/* Round constant RC[i]: bit (2^j - 1) is rc(j + 7i) for j = 0..l,
 * all other bits are 0. */
static uint64_t rc_const(int i)
{
    uint64_t v = 0;
    int j;

    for (j = 0; j <= KECCAK_L; j++)
        if (rc(j + 7 * i))
            v |= (uint64_t)1 << ((1 << j) - 1);
    return v;
}

int main(void)
{
    int r[5][5];
    int t, x, y, nx, ny, i;

    // Algorithm 2: walk over the matrix, starting at (1,0)
    x = 1;
    y = 0;
    for (t = 0; t < KECCAK_ROUNDS; t++)
    {
        r[x][y] = ((t + 1) * (t + 2) / 2) % 64;
        nx = y;
        ny = (2 * x + 3 * y) % 5;
        x = nx;
        y = ny;
    }
    r[0][0] = 0;

    printf("/* Generated from FIPS 202, Algorithms 2 and 5. */\n\n");

    printf("static const uint64_t RC[KECCAK_ROUNDS] = {\n");
    for (i = 0; i < KECCAK_ROUNDS; i++)
        printf("    0x%016llxULL, /* RC[%2d] */\n",
               (unsigned long long)rc_const(i), i);
    printf("};\n\n");

    printf("static const unsigned int RHO[KECCAK_LANES] = {\n");
    for (y = 0; y < 5; y++)
    {
        printf("    ");
        for (x = 0; x < 5; x++)
            printf("%2d, ", r[x][y]);
        printf("/* y = %d : (x = 0..4) */\n", y);
    }
    printf("};\n");

    return 0;
}

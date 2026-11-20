#ifndef SHA3_H
#define SHA3_H

#include <stddef.h>
#include <stdint.h>

#include "keccak.h"

#define SHA3_256_RATE 136 // (b: 1600 - r: 512) / 8
#define SHA3_256_DIGEST 32 // 256 / 8

typedef struct
{
    uint64_t state[KECCAK_LANES]; // the 1600-bit sponge state
    uint8_t buf[SHA3_256_RATE]; // pending input, less than one block
    size_t buflen; // number of bytes in buf
} sha3_256_ctx;

void sha3_256_init(sha3_256_ctx *ctx);
void sha3_256_update(sha3_256_ctx *ctx, const void *data, size_t len);
void sha3_256_final(sha3_256_ctx *ctx, uint8_t out[SHA3_256_DIGEST]);

void sha3_256(const void *data, size_t len, uint8_t out[SHA3_256_DIGEST]);

#endif /* !SHA3_H */

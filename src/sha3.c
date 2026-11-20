#include "sha3.h"

#include <stddef.h>
#include <string.h>

/* Pad10*1 with the SHA-3 domain suffix 01
 *
 * Be aware this is not Keccak-256's padding, but SHA-3's */
static void pad_block(sha3_256_ctx *ctx)
{
    memset(ctx->buf + ctx->buflen, 0, SHA3_256_RATE - ctx->buflen);
    ctx->buf[ctx->buflen] ^= 0x06;
    ctx->buf[SHA3_256_RATE - 1] ^= 0x80;
}

/* XOR one block into the rate part of the state, then apply the permutation */
static void absorb_block(sha3_256_ctx *ctx, const uint8_t block[SHA3_256_RATE])
{
    size_t i;

    for (i = 0; i < SHA3_256_RATE; i++)
        ctx->state[i / 8] ^= (uint64_t)block[i] << (8 * (i % 8));

    keccak_f1600(ctx->state);
}

void sha3_256_init(sha3_256_ctx *ctx)
{
    memset(ctx, 0, sizeof *ctx);
}

/* Feeds `len` bytes from `data` into the hash context */
void sha3_256_update(sha3_256_ctx *ctx, const void *data, size_t len)
{
    const uint8_t *input = data;
    size_t remaining = len;

    while (remaining > 0)
    {
        // how many bytes still fit in the block buffer
        size_t room = SHA3_256_RATE - ctx->buflen;

        // Copy as much as possible if not all
        size_t chunk_len = (remaining < room) ? remaining : room;

        memcpy(ctx->buf + ctx->buflen, input, chunk_len);
        ctx->buflen += chunk_len;
        input += chunk_len;
        remaining -= chunk_len;

        // Buffer full: process one block then start a new one
        if (ctx->buflen == SHA3_256_RATE)
        {
            absorb_block(ctx, ctx->buf);
            ctx->buflen = 0;
        }
    }
}

/* Finishes the hash (last block) and writes the 32-byte digest to `out`
 *
 * Must be called once per context after the last call to sha3_256_update() */
void sha3_256_final(sha3_256_ctx *ctx, uint8_t out[SHA3_256_DIGEST])
{
    size_t i;

    // Padding then Absorbing last block
    pad_block(ctx);
    absorb_block(ctx, ctx->buf);

    // Squeezing: first 256 bits of the rate
    for (i = 0; i < SHA3_256_DIGEST; i++)
        out[i] = (uint8_t)(ctx->state[i / 8] >> (8 * (i % 8)));
}

void sha3_256(const void *data, size_t len, uint8_t out[SHA3_256_DIGEST])
{
    sha3_256_ctx ctx;

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, data, len);
    sha3_256_final(&ctx, out);
}

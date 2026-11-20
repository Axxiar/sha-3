#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sha3.h"

static int failures;

static void to_hex(const uint8_t d[SHA3_256_DIGEST], char out[65])
{
    int i;

    for (i = 0; i < SHA3_256_DIGEST; i++)
        snprintf(out + 2 * i, 3, "%02x", d[i]);
    out[64] = '\0';
}

/* Compares a digest with an expected hex string. */
static void check_hex(const char *name, const uint8_t d[SHA3_256_DIGEST],
                      const char *expected)
{
    char got[65];

    to_hex(d, got);
    if (strcmp(got, expected) == 0)
    {
        printf("+OK   %s\n", name);
    }
    else
    {
        printf("-FAIL %s\n  got      %s\n  expected %s\n", name, got, expected);
        failures++;
    }
}

/* Known answer tests. */
static void test_vectors(void)
{
    uint8_t d[SHA3_256_DIGEST];

    sha3_256("", 0, d);
    check_hex(
        "empty message", d,
        "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a");

    sha3_256("abc", 3, d);
    check_hex(
        "\"abc\"", d,
        "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532");
}

/* Self-consistency: for each length, hashing in one call must equal
 * hashing as two updates split at every possible position. This exercises
 * the buffer logic around the 136-byte block boundary. */
static void test_split_consistency(void)
{
    uint8_t msg[300];
    uint8_t ref[SHA3_256_DIGEST], d[SHA3_256_DIGEST];
    sha3_256_ctx ctx;
    size_t len, k, i;
    int bad = 0;

    for (i = 0; i < sizeof msg; i++)
        msg[i] = (uint8_t)(i * 7 + 3);

    for (len = 0; len <= sizeof msg; len++)
    {
        sha3_256(msg, len, ref);

        for (k = 0; k <= len; k++)
        {
            sha3_256_init(&ctx);
            sha3_256_update(&ctx, msg, k);
            sha3_256_update(&ctx, msg + k, len - k);
            sha3_256_final(&ctx, d);

            if (memcmp(ref, d, SHA3_256_DIGEST) != 0)
            {
                printf("-FAIL split consistency: len %zu, split at %zu\n", len,
                       k);
                bad = 1;
                break;
            }
        }
        if (bad)
            break;
    }

    if (!bad)
        printf("+OK   split consistency, lengths 0..%zu\n", sizeof msg);
    else
        failures++;
}

/* Byte-by-byte feeding must match one-shot hashing. */
static void test_byte_by_byte(void)
{
    uint8_t msg[300];
    uint8_t ref[SHA3_256_DIGEST], d[SHA3_256_DIGEST];
    sha3_256_ctx ctx;
    size_t len, i;
    int bad = 0;

    for (i = 0; i < sizeof msg; i++)
        msg[i] = (uint8_t)(i * 13 + 1);

    for (len = 0; len <= sizeof msg && !bad; len++)
    {
        sha3_256(msg, len, ref);

        sha3_256_init(&ctx);
        for (i = 0; i < len; i++)
            sha3_256_update(&ctx, msg + i, 1);
        sha3_256_final(&ctx, d);

        if (memcmp(ref, d, SHA3_256_DIGEST) != 0)
        {
            printf("-FAIL byte-by-byte: len %zu\n", len);
            bad = 1;
        }
    }

    if (!bad)
        printf("+OK   byte-by-byte, lengths 0..%zu\n", sizeof msg);
    else
        failures++;
}

int main(void)
{
    test_vectors();
    test_split_consistency();
    test_byte_by_byte();

    if (failures == 0)
    {
        puts("ALL OK");
        return 0;
    }
    printf("%d test(s) failed\n", failures);
    return 1;
}

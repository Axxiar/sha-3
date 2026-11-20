#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sha3.h"

/* Bytes read by fread: 480 blocks of 136 bytes (~ 64 KB)
 * Using a multiple of the rate keeps the input aligned */
#define READ_SIZE (SHA3_256_RATE * 480)

static uint8_t read_buf[READ_SIZE];

int usage(char *program_name)
{
    fprintf(stderr, "Usage: %s [file]\n", program_name);
    return 1;
}

/* Hashes everything readable from `in` and prints "<digest>  <name>".
 * Returns 0 on success, -1 if the stream reported an error. */
static int hash_stream(FILE *in, const char *name)
{
    sha3_256_ctx ctx;
    uint8_t digest[SHA3_256_DIGEST];
    size_t n, i;

    sha3_256_init(&ctx);

    while ((n = fread(read_buf, 1, READ_SIZE, in)) > 0)
        sha3_256_update(&ctx, read_buf, n);

    if (ferror(in))
        return -1;

    sha3_256_final(&ctx, digest);

    for (i = 0; i < SHA3_256_DIGEST; i++)
        printf("%02x", digest[i]);
    printf("  %s\n", name);

    return 0;
}

int main(int argc, char **argv)
{
    const char *name = "-";
    int status;

    if (argc > 2)
    {
        fprintf(stderr, "Invalid number of parameters\n");
        return usage(argv[0]);
    }

    FILE *in = stdin; // default to stdin if no arg
    if (argc == 2 && strcmp(argv[1], "-") != 0)
    {
        name = argv[1];
        in = fopen(name, "rb");
        if (in == NULL)
        {
            perror(name);
            return 1;
        }
    }

    status = hash_stream(in, name);

    if (in != stdin)
        fclose(in);

    if (status != 0)
    {
        fprintf(stderr, "%s: read error\n", name);
        return 1;
    }
    return 0;
}

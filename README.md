# SHA-3-256 implementation in C

SHA-3-256 is an instance of the Keccak sponge construction (FIPS 202) built on the `Keccak-f[1600]` permutation.

## Specifications and implementation

> [!IMPORTANT]
> The algorithm specifications and parameters below are taken from the official paper [NIST FIPS 202](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf)
>
> The `Design choices` subsection also documents my own implementation decisions

Across all fixed-length SHA-3 variants (224, 256, 384, and 512), the capacity `c` is set to twice the digest length `d`. For SHA-3-256 (`d = 256` bits), this gives the following parameters matching the constants in `include/keccak.h` and `include/sha3.h`:

- **State width (`b`)**: 1600 bits (5 x 5 array of 64-bit lanes, `w = 64`, `l = 6`)
- **Rounds (`n_r`)**: 24 (`12 + 2l`), each applying `theta`, `rho`, `pi`, `chi`, and `iota`
- **Capacity (`c`)**: `2 * 256 = 512` bits
- **Rate (`r`)**: `b - c = 1600 - 512 = 1088` bits 
- **Domain separation and padding**: the 2-bit SHA-3 suffix `01` is appended to the message before multi-rate padding `pad10*1`, and a single 256-bit squeeze produces the final digest.

### Design choices

#### State and endianness

The 1600-bit state is stored as 25 `uint64_t` lanes (`src/keccak.c`). Bytes are XORed into lanes and extracted with little-endian shifts (`8 * (i % 8)`), so the result does not depend on host endianness or unaligned casts.

The `RC` and `RHO` tables are precomputed with `tools/compute_constants.c` (Algorithms 2 and 5 of FIPS 202).

#### Block chaining and padding

`sha3_256_ctx` (`include/sha3.h`) keeps a 136-byte (1088 / 8) block buffer `buf` and its current length `buflen`.

`sha3_256_update` fills `buf` and absorbs a block with `keccak_f1600` each time it reaches 136 bytes.

`sha3_256_final` pads the remaining bytes using the multi-rate padding `pad10*1` algorithm and SHA-3's suffix (`01`).

#### Memory and file I/O

Nothing is heap-allocated. `src/main.c` opens files in binary mode (`"rb"`) and reads into a static buffer of `480 * 136 = 65280` bytes (~64 KB), so memory usage stays flat regardless of input size.

Hashing a 50 MB file takes under a second compiled in C99 with `-O2`.

> [!NOTE]
> The code is adapted to be portable to OpenBSD.

## Usage

### Command-line tool (`sha3sum`)

```sh
make
./sha3sum [file]   # reads stdin if no file or "-" is passed
```

### As a C library

Include `sha3.h` and compile `src/sha3.c` and `src/keccak.c` with `-Iinclude`:

```c
#include "sha3.h"

uint8_t digest[SHA3_256_DIGEST];

/* One-shot buffer */
sha3_256(data, len, digest);

/* Streaming */
sha3_256_ctx ctx;
sha3_256_init(&ctx);
sha3_256_update(&ctx, chunk1, len1);
sha3_256_update(&ctx, chunk2, len2);
sha3_256_final(&ctx, digest);
```

### Regenerating constants

`make compute-constants` builds and runs `tools/compute_constants.c`, printing the `RC` round constants and `RHO` rotation offsets to `stdout`. This way you can see how they were calculated.

## Tests

`make test` compiles with `-O2 -Wall -Wextra -pedantic -std=c99` and runs three checks:

- `test_keccak`: checks `keccak_f1600` from the all-zero state over two consecutive permutations against XKCP intermediate values.
- `test_sha3`: checks SHA3-256 known-answer vectors (empty, `"abc"`, and multi-block 448-, 896-, and 1600-bit inputs), then verifies that for every message length in `0..300`, one-shot hashing matches two-part splits at every offset as well as byte-by-byte updates across the 136-byte rate boundary.
- `test/test.sh`: compares `./sha3sum` against `openssl dgst -sha3-256` on one 50 MB random file, 100 random 50 MB files in sequence, and boundary file sizes (`0, 135, 136, 137, 272, 273` bytes).

> [!Note]
> An LLM was used to help write part of the test suite and this README. All test cases and vectors were reviewed and verified against NIST/XKCP references and `openssl`.

## References

- NIST FIPS 202: https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf
- Keccak Team: https://keccak.team/
- Wikipedia (SHA-3): https://en.wikipedia.org/wiki/SHA-3

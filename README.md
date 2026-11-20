# SHA-3-256 implementation in C

> [!NOTE]
> All implementation documentation is on [this pdf](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf) available at https://csrc.nist.gov/pubs/fips/202/final

SHA-3-256 relies on the Keccak-f[1600] permutation.

Capacity is always 2* the digest (whichever implementation between SHA3- 224,256,384,512)
So in our case it will be:

```
Permutation width = 1600
capacity = 2*256 = 512
rate = 1600 - 512 = 1088
```

SHA3-256(M) = KECCAK\[512] (M || 01, 256);

KECCAK\[c] (N, d) = SPONGE\[KECCAK-p\[1600, 24], pad10\*1, 1600 – c] (N, d).
here -> KECCAK\[512] (N, 256) = SPONGE\[KECCAK-p\[1600, 24], pad10\*1, 1088] (N, 256).

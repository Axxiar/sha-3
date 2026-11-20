#!/bin/sh
# Compares sha3sum with openssl on one 50 MB file, then on 100 files of 50 MB.
# Usage: sh test/test.sh   (needs about 100 MB of free space in /tmp)

cd "$(dirname "$0")/.." || exit 1
make sha3sum >/dev/null || exit 1

tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT INT TERM

# --- 1. one 50 MB file ----------------------------------------------------
fail=0
dd if=/dev/urandom of="$tmp/big.bin" bs=1048576 count=50 2>/dev/null

a=$(./sha3sum "$tmp/big.bin" | awk '{print $1}')
b=$(openssl dgst -sha3-256 "$tmp/big.bin" | awk '{print $NF}')
[ "$a" = "$b" ] || { echo "MISMATCH on the 50 MB file"; fail=1; }

# --- 2. 100 hashes of 50 MB, one file on disk at a time -------------------
i=1
while [ $i -le 100 ]; do
    dd if=/dev/urandom of="$tmp/f.bin" bs=1048576 count=50 2>/dev/null
    a=$(./sha3sum "$tmp/f.bin" | awk '{print $1}')
    b=$(openssl dgst -sha3-256 "$tmp/f.bin" | awk '{print $NF}')
    [ "$a" = "$b" ] || { echo "MISMATCH on file $i"; fail=1; }
    i=$((i + 1))
done

# --- 3. sizes around the block boundary (rate = 136 bytes) ----------------
for n in 0 135 136 137 272 273; do
    dd if=/dev/urandom of="$tmp/s.bin" bs=1 count="$n" 2>/dev/null
    a=$(./sha3sum "$tmp/s.bin" | awk '{print $1}')
    b=$(openssl dgst -sha3-256 "$tmp/s.bin" | awk '{print $NF}')
    [ "$a" = "$b" ] || { echo "MISMATCH on a $n-byte file"; fail=1; }
done

[ $fail -eq 0 ] && echo "all OK" || echo "FAILED"
exit $fail

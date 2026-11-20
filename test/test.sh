#!/bin/sh
# Compares sha3sum with openssl on one 50 MB file, then on 100 files of 50 MB.
# Usage: sh test/run_big.sh   (needs about 100 MB of free space in /tmp)

cd "$(dirname "$0")/.." || exit 1
make sha3sum >/dev/null || exit 1

tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT INT TERM

# --- 1. one 50 MB file ----------------------------------------------------
dd if=/dev/urandom of="$tmp/big.bin" bs=1m count=50 2>/dev/null

time ./sha3sum "$tmp/big.bin"
openssl dgst -sha3-256 "$tmp/big.bin"

# --- 2. 100 hashes of 50 MB, one file on disk at a time -------------------
fail=0
i=1
while [ $i -le 100 ]; do
    dd if=/dev/urandom of="$tmp/f.bin" bs=1m count=50 2>/dev/null
    a=$(./sha3sum "$tmp/f.bin" | awk '{print $1}')
    b=$(openssl dgst -sha3-256 "$tmp/f.bin" | awk '{print $NF}')
    [ "$a" = "$b" ] || { echo "MISMATCH on file $i"; fail=1; }
    i=$((i + 1))
done

[ $fail -eq 0 ] && echo "100/100 OK" || echo "FAILED"
exit $fail

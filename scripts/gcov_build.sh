#!/usr/bin/env bash
# Build test_fn/test_objects with gcov instrumentation, run them (both must
# pass), dump gcov JSON to build/gcovj and write work/gcov_exec.tsv.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
SRC=$(make -s --eval='printsrc: ; @echo $(SRC)' printsrc)
rm -rf build/gcovj build/*.gcda build/*.gcno; mkdir -p build/gcovj
for t in test_fn test_objects; do
  gcc -std=c11 -O1 --coverage -w -o build/$t.cov tests/$t.c $SRC &
done; wait
./build/test_fn.cov 2>&1 | grep -v '^f3mem' | tail -1 | grep -q PASS || { echo "test_fn failed"; exit 1; }
./build/test_objects.cov > build/test_objects.cov.log 2>&1 || { echo "test_objects failed"; exit 1; }
(cd build/gcovj && ls ../*.gcda | xargs -P "$(nproc)" -n 40 gcov --json-format -o .. >/dev/null 2>&1)
python3 scripts/gcov_exec.py

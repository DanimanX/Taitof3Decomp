#!/usr/bin/env bash
# Build every differential test suite (tests/test_*.c, all checked against
# MAME captures) with gcov instrumentation, run them (all must pass), dump
# gcov JSON to build/gcovj and write work/gcov_exec.tsv.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
SUITES="fn objects kernel player player_task collision sound objpool stage sprite"
rm -rf build/gcovj build/*.gcda build/*.gcno; mkdir -p build/gcovj
for t in $SUITES; do
  # the suite's own compile command from the Makefile, instrumented
  cmd=$(make -s -B -n CC=gcc CFLAGS="-std=c11 -O1 --coverage -w" build/test_$t | grep -E -- "-o build/test_$t( |$)" | tail -1)
  cmd=${cmd/-o build\/test_$t /-o build/test_$t.cov }
  eval "$cmd" &
done; wait
fail=0
for t in $SUITES; do
  if [ $t = fn ]; then
    ./build/test_fn.cov 2>&1 | grep -v '^f3mem' | tail -1 | grep -q PASS || { echo "test_fn failed"; fail=1; }
  else
    ./build/test_$t.cov > build/test_$t.cov.log 2>&1 || { echo "test_$t failed"; fail=1; }
  fi
done
[ $fail = 0 ] || exit 1
(cd build/gcovj && ls ../*.gcda | xargs -P "$(nproc)" -n 40 gcov --json-format -o .. >/dev/null 2>&1)
python3 scripts/gcov_exec.py

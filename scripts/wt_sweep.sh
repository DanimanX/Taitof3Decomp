#!/usr/bin/env bash
# Test a worktree's uncommitted routine batch and print an exact verdict per
# routine. Usage: scripts/wt_sweep.sh <worktree-dir>   (VERDICTS=<file> sets the log,
# default sweep.tsv in the worktree root; it is appended to)
# Prints "ADDR N/N un=U state=S spr=SPR gfx=G" for every obj_ADDR.c modified or
# added in the worktree (batch mode: iterates fast; per-routine pass: only the
# routines that can still fail).
set -uo pipefail
WT=$1
cd "$WT" || exit 2
make -s build/test_objects >/dev/null 2>&1 || { echo "BUILD FAILED"; make build/test_objects 2>&1 | grep -E "error" | head -5; }
files=$( { git status --short | awk '{print $2}'; git diff --name-only HEAD; } | grep -oE 'src/enemy/obj_[0-9A-F]+\.c$' | sort -u )
for f in $files; do
  a=$(basename "$f" .c); a=${a#obj_}
  r=$(F3_ONLY=$a timeout 90 ./build/test_objects 2>&1 | grep -E "0x0*$a\b" | tail -1)
  n=$(echo "$r" | grep -oE '[0-9]+/ *[0-9]+' | head -1 | tr -d ' ')
  un=$(echo "$r" | grep -oE '[0-9]+ reach undecompiled' | cut -d' ' -f1)
  st=$(echo "$r" | grep -oE '[0-9]+ state changes' | cut -d' ' -f1)
  sp=$(echo "$r" | grep -oE '[0-9]+ alloc/free' | cut -d' ' -f1)
  gfx=$([ -e "work/ram/obj_${a}_gfx.bin" ] && echo 1 || echo 0)
  echo "${a:-?} ${n:-NONE} un=${un:-?} state=${st:-0} alloc=${sp:-0} gfx=$gfx" >> "${VERDICTS:-sweep.tsv}"
done

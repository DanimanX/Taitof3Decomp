#!/usr/bin/env bash
# Capture one object tick routine at the pool call site, trying savestates
# until enough ticks are seen. Usage: scripts/capture_obj.sh <ADDR> [n=200]
# -> work/ram/obj_<ADDR>_{pairs,spr,regs}; prints "<ADDR> <state> pairs=N"
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
A=$(echo "$1" | tr a-z A-Z); N=${2:-200}; a=$(echo "$A" | tr A-Z a-z | sed 's/^0*//')
best=0
for st in 02_stage1_start 03_active_combat 01_attract; do
  # F3_SAMPLES must match the n we intend to keep: capture_fn.sh defaults it to
  # 300, so passing n only to the exit list would make obj_<A>_pairs.bin a
  # silent PREFIX of what was captured (no way to tell 60 of 60 from 60 of 300).
  out=$(F3_SPR=1 F3_SAMPLES="$N" F3_COND="a1==$a" F3_MAXFRAMES=${F3_MAXFRAMES:-6000} timeout -s KILL 400 \
        "$ROOT/scripts/capture_fn.sh" 104EE 104F0 "$N" "$ROOT/work/ram/obj_${A}_try" "$st" 2>&1 | grep -oE "pairs=[0-9]+" | cut -d= -f2)
  out=${out:-0}
  if [ "$out" -gt "$best" ]; then
    best=$out
    for f in "$ROOT"/work/ram/obj_${A}_try_*; do mv "$f" "${f/_try/}"; done
    beststate=$st
  fi
  rm -f "$ROOT"/work/ram/obj_${A}_try_*
  [ "$best" -ge 50 ] && break
done
echo "$A ${beststate:-none} pairs=$best"

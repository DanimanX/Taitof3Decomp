#!/usr/bin/env bash
# Capture one object tick routine at the pool call site, trying savestates
# until enough ticks are seen. Usage: scripts/capture_obj.sh <ADDR> [n=200]
# -> work/ram/obj_<ADDR>_{pairs,spr,regs}; prints "<ADDR> <state> pairs=N"
#
# For "lifespan tail" routines the interesting tick is the one where a timer
# crosses zero, which the plain run almost never reaches. Pass the poke env
# through to capture_fn.lua to force it, e.g.
#   F3_POKEFN=<addr> F3_POKEA0B=3e=1 scripts/capture_obj.sh <addr> 60
# F3_POKEFN makes the poke apply only while A1 is this routine, so other
# objects ticking in the same frame are untouched. The poke lands before the
# "pre" snapshot, so the recorded pair is self-consistent.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
A=$(echo "$1" | tr a-z A-Z); N=${2:-200}; STATE=${3:-}; a=$(echo "$A" | tr A-Z a-z | sed 's/^0*//')
best=0
# Stash the existing capture so a fruitless re-run cannot destroy it.
if [ -s "$ROOT/work/ram/obj_${A}_pairs.bin" ]; then
  cp -f "$ROOT/work/ram/obj_${A}_pairs.bin" "$ROOT/work/ram/obj_${A}_pairs.bin.good"
fi
# A named state overrides the default sweep: the three stock states no longer
# reproduce the frames the older captures came from, so a re-capture has to
# name the state the routine was actually alive in.
if [ -n "$STATE" ]; then STATES="$STATE"; else STATES="02_stage1_start 03_active_combat 01_attract"; fi
# F3_NOFFSET keeps an input pattern in phase with the run that saved the
# state, but the survey saves at frame N of *its* own run, and applying the
# offset when re-loading that state desynchronises it (it stops finding the
# routine). Only set it when the caller asks.
if [ "${F3_USE_NOFFSET:-0}" = 1 ]; then
  case "$STATE" in *_[0-9][0-9][0-9][0-9][0-9][0-9]) export F3_NOFFSET=$((10#${STATE##*_}));; esac
fi
for st in $STATES; do
  # F3_SAMPLES must match the n we intend to keep: capture_fn.sh defaults it to
  # 300, so passing n only to the exit list would make obj_<A>_pairs.bin a
  # silent PREFIX of what was captured (no way to tell 60 of 60 from 60 of 300).
  out=$(F3_SPR=1 F3_SAMPLES="$N" F3_COND="a1==$a" F3_MAXFRAMES=${F3_MAXFRAMES:-6000} \
        F3_POKEFN="${F3_POKEFN:-}" F3_POKEA0B="${F3_POKEA0B:-}" \
        F3_POKEA0P="${F3_POKEA0P:-}" F3_POKE32="${F3_POKE32:-}" \
        F3_POKE16="${F3_POKE16:-}" \
        timeout -s KILL 400 \
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
# Never leave a thin capture in place of a good one: a run that finds nothing
# still truncates work/ram/obj_<A>_pairs.bin, because capture_fn.sh opens it
# directly. Guard the old capture before the attempts and put it back if this
# invocation did not beat it.
if [ "$best" -lt 1 ]; then
  if [ -s "$ROOT/work/ram/obj_${A}_pairs.bin.good" ]; then
    mv -f "$ROOT/work/ram/obj_${A}_pairs.bin.good" "$ROOT/work/ram/obj_${A}_pairs.bin"
    echo "$A ${beststate:-none} pairs=0 (restored previous capture)"
    exit 0
  fi
  rm -f "$ROOT/work/ram/obj_${A}_pairs.bin"
fi
echo "$A ${beststate:-none} pairs=$best"

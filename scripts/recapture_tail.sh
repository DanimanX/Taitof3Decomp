#!/usr/bin/env bash
# Find which savestate actually contains a routine, then recapture it there.
#
#   scripts/recapture_tail.sh <ADDR> [states_glob]
#
# Scans the candidate states for one where the routine ticks, then re-runs the
# capture on that state with the punch poke applied (default: force +0x3E to 1,
# so a `subq.w #1,(0x3E,A0) ; bgt` tail falls through to its exit next tick).
#
# Env:
#   POKE   "off=val" pairs for F3_POKEA0W (default "6=1")
#   GRAB   frames to sample per state before giving up (default 3000)
#   KEEP   "1" to keep the previous obj_<ADDR>_* capture as *.bak
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
A=$(echo "${1:?usage: recapture_tail.sh <ADDR> [glob]}" | tr a-z A-Z)
a=$(echo "$A" | tr A-Z a-z | sed 's/^0*//')
GLOB=${2:-"$ROOT/states/dariusg/LA_*.sta"}
GRAB=${GRAB:-3000}
POKE=${POKE:-6=1}

found=""
for f in $GLOB; do
  st=$(basename "$f" .sta)
  off=$(echo "$st" | grep -oE '[0-9]{4,6}$' || echo 0)
  r=$(F3_SAMPLES=2 F3_COND="a1==$a" F3_MAXFRAMES="$GRAB" \
      timeout -s KILL 120 "$ROOT/scripts/capture_fn.sh" 104EE 104F0 2 \
      "$ROOT/work/ram/rc_try" "$st" 2>&1 | grep -oE "pairs=[0-9]+")
  echo "  $st -> ${r:-pairs=0}"
  case "${r:-}" in pairs=0|"") ;; *) found=$st; break ;; esac
done

if [ -z "$found" ]; then
  echo "$A none-of-$(basename "$(dirname "${GLOB%%\**}")")"; exit 1
fi

out=$(F3_SAMPLES=60 F3_COND="a1==$a" F3_MAXFRAMES=6000 \
      F3_POKEFN="$a" F3_POKEA0W="$POKE" F3_POKECAPTURE=1 \
      timeout -s KILL 400 "$ROOT/scripts/capture_obj.sh" "$A" 60 "$found" 2>&1 | tail -1)
echo "$A via $found poke=$POKE -> $out"

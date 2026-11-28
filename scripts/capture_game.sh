#!/usr/bin/env bash
# Capture one routine of a second game: <game> <addr>. Tries the states in
# CAP_STATES (default: P_play3500 A_attract1000 P_select) until one gives
# pairs; output <game>/work/ram/fn_<addr>_*; prints "<addr> <state> pairs=N".
# Env: CAP_FRAMEVAR (frame counter, default elvactr's 400142).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
G=$1 A=$2
for st in ${CAP_STATES:-P_play3500 A_attract1000 P_select}; do
  rm -f $G/work/ram/fn_${A}_*
  r=$(F3_GAME=$G F3_FRAMEVAR=${CAP_FRAMEVAR:-400142} F3_GFX=1 F3_SPR=1 F3_DPRAM=1 F3_MAXFRAMES=${CAP_MAXF:-1500} \
      timeout -s KILL 900 scripts/capture_fn.sh $A ret ${CAP_N:-30} $G/work/ram/fn_$A $st 2>&1 | grep -oE "pairs=[0-9]+")
  n=${r#pairs=}
  if [ "${n:-0}" -gt 0 ]; then echo "$A $st pairs=$n"; exit 0; fi
done
rm -f $G/work/ram/fn_${A}_*
echo "$A - pairs=0"

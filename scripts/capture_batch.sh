#!/usr/bin/env bash
# Capture one object routine from a survey row: <FN> <state> <first_frame> <count> <group>
# (work/survey/todo.tsv). Group 11 objects are ticked from 0x10568, the rest
# from 0x104EE. Output work/ram/obj_<FN>_*; prints "<FN> pairs=N".
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FN=$1 ST=$2 FIRST=$3 G=$5
fn=$(echo "$FN" | tr A-Z a-z | sed 's/^0*//')
if [ "$G" = 11 ]; then EN=10568 EX=1056A; else EN=104EE EX=104F0; fi
MAXF=$(( FIRST + 3000 ))
r=$(F3_SPR=1 F3_COND="a1==$fn" F3_MAXFRAMES=$MAXF timeout -s KILL 1500 \
    "$ROOT/scripts/capture_fn.sh" $EN $EX "${N:-100}" "$ROOT/work/ram/obj_$FN" "$ST" 2>&1 | grep -oE "pairs=[0-9]+")
echo "$FN $ST ${r:-pairs=0}"

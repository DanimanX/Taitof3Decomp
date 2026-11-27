#!/usr/bin/env bash
# Usage: scripts/seg_capture.sh <resume_hex> <yields_hex,...> <zone> <frame> [samples]
# Capture segments of task code (see src/kernel/seg.h): each pair starts at
# <resume> (the instruction after a TRAP #4/#5, or a routine entry) and ends
# at the first of <yields> (every TRAP #4/#5 the code can reach) executed.
# Interrupts are masked inside a pair (F3_NOIRQ, F3_REMASK=16A4) and the exit
# PC is recorded (F3_EXITPC) so test_fn can check which yield was reached.
# Starts from zone state Z<zone> near survey frame <frame> as zcap_one.sh.
# Output: work/ram/seg_<resume>_*.  Prints: resume zone state pairs=N.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
R=$1 Y=$2 Z=$3 FR=$4 N=${5:-20}
abs=$((1000 + FR)); s=$(( abs / 1000 * 1000 )); [ $s -lt 1000 ] && s=1000; [ $s -gt 25000 ] && s=25000
st=$(printf "Z%s_%06d" "$Z" $s)
r=$(F3_EXITPC=1 F3_NOIRQ=1 F3_REMASK=16A4 F3_GFX=1 F3_SPR=1 F3_SAMPLES=$N \
    F3_MAXFRAMES=$((26000 - s)) F3_NOFFSET=$s F3_INPUT=fire F3_POKE16=41B1E0=00D2 \
    timeout -s KILL ${SEG_TIMEOUT:-1500} scripts/capture_fn.sh "$R" "$Y" "$N" "work/ram/seg_$R" "$st" 2>&1 \
    | grep -oE "pairs=[0-9]+" | tail -1)
r=${r:-pairs=0}
[ "$r" = pairs=0 ] && find work/ram -maxdepth 1 -name "seg_${R}_*" -delete
echo -e "$R\tZ$Z\t$st\t$r"

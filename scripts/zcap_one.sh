#!/usr/bin/env bash
# zcap_one.sh ADDR ZONE FRAME : capture 20 calls of ADDR from zone state ZONE,
# starting from the save just before FRAME (frame counted from Z<zone>_001000,
# as work/zcap/hitsz reports it). Post state = return of the SAME call
# (F3_EXIT=ret, SP-matched). Prints: ADDR ZONE STATE pairs=N secs
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
A=$1 Z=$2 FR=$3
abs=$((1000 + FR)); s=$(( abs / 1000 * 1000 )); [ $s -lt 1000 ] && s=1000; [ $s -gt 25000 ] && s=25000
st=$(printf "Z%s_%06d" "$Z" $s)
t0=$(date +%s)
r=$(F3_GFX=1 F3_SPR=1 F3_SAMPLES=${ZCAP_SAMPLES:-20} F3_MAXFRAMES=$((26000 - s)) F3_NOFFSET=$s F3_INPUT=fire \
    F3_POKE16=41B1E0=00D2${ZCAP_POKE16:+,$ZCAP_POKE16} timeout -s KILL ${ZCAP_TIMEOUT:-1200} \
    scripts/capture_fn.sh "$(echo "$A" | sed 's/^0*//')" ret ${ZCAP_SAMPLES:-20} "${ZCAP_OUT:-work/ram/fn_$A}" "$st" 2>&1 | grep -oE "pairs=[0-9]+" | tail -1)
r=${r:-pairs=0}
[ "$r" = pairs=0 ] && find work/ram -maxdepth 1 -name "$(basename "${ZCAP_OUT:-fn_$A}")_*" -size 0 -delete
echo -e "$A\t$Z\t$st\t$r\t$(( $(date +%s) - t0 ))s"

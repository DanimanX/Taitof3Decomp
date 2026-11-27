#!/usr/bin/env bash
# check_capture.sh ADDR PREFIX: run the translated Fn_ADDR (from build/libf3.a)
# and an empty Fn against capture work/ram/PREFIX (e.g. fni_0D4FF6).
# Prints "ADDR real=a/b null=c/d"; the capture is strong when real passes
# all and null fails at least one.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
A=$1 P=$2 T=chk$A
for f in work/ram/${P}_*; do ln -sf "$(basename "$f")" "work/ram/fn_${T}${f#work/ram/$P}"; done
D=build/chk; mkdir -p $D
printf '#include "fn_auto.h"\nint Fn_null_%s(const f3mem_t *m, f3regs_t *r) { (void)m; (void)r; return 0; }\n' $A > $D/null_$A.c
run() { cc -std=c11 -O1 -w -iquote src/auto -DFN1=$1 -DFN1_ADDR="\"$T\"" -o $D/t_${A}_$2 tests/test_fn.c $3 build/libf3.a 2>&1 | head -3
        timeout 300 $D/t_${A}_$2 2>&1 | grep -oE "[0-9]+/ *[0-9]+ calls" | head -1 | tr -d ' ' | sed 's/calls//'; }
r=$(run Fn_$A real ""); n=$(run Fn_null_$A null $D/null_$A.c)
rm -f work/ram/fn_${T}_*
echo "$A real=$r null=$n"

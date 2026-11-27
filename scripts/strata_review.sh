#!/usr/bin/env bash
# strata_review.sh [--watch] [COMMIT...] : first-pass review, by the local Strata
# model, of the routines added to src/auto by the given commits (default: the
# "feat: ... routines from ... round" commits not yet reviewed). Each routine's C
# is read next to its disassembly bundle and Strata is asked for paths the C
# skips or gets wrong, which a 1-2 call capture may not exercise.
# Output: work/zcap/strata_review.tsv  (addr, commit, OK|SUSPECT|ERROR, note)
#         work/zcap/strata_review/ADDR.txt  (full answer)
# Every SUSPECT is a hypothesis for Claude to check against the ROM.
# --watch: keep running, checking for new commits every 5 minutes.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
OUT=work/zcap/strata_review.tsv; DIR=work/zcap/strata_review; mkdir -p "$DIR"; touch "$OUT"
WATCH=0; [ "${1:-}" = --watch ] && { WATCH=1; shift; }

Q='You review a C translation of a 68020 ROM routine against its disassembly (the routine first, then its callees; "STOP" callees are already decompiled C helpers). The C was tested bit-exact on only 1-2 captured calls, so branches those calls did not take are unverified.
Check every conditional branch, jump table entry and callee of the ROUTINE (not of STOP callees): is each path present in the C with the same condition (signed/unsigned, size .b/.w/.l), the same constants and offsets, and the same writes? A "return -1" is acceptable only for a path that calls something not translated, and only before that path writes memory.
Answer on the FIRST line with exactly "VERDICT: OK" or "VERDICT: SUSPECT", then at most 6 short lines, each "<ROM address>: <what differs>". Do not list style issues.'

review() { # commit
  local c=$1 a f v note
  for f in $(git show --name-only --diff-filter=A --format= "$c" -- 'src/auto/fn_*.c'); do
    a=$(basename "$f" .c); a=${a#fn_}
    grep -q "^$a	" "$OUT" && continue
    [ -f "$f" ] || continue
    local bnd=work/disasm/bundle_$(echo "$a" | sed 's/^0*//').s
    [ -f "$bnd" ] || { printf '%s\t%s\tERROR\tno bundle\n' "$a" "${c:0:7}" >> "$OUT"; continue; }
    if ! strata-local ask -f "$f" -f "$bnd" "$Q" > "$DIR/$a.txt" 2>&1; then
      printf '%s\t%s\tERROR\tstrata ask failed\n' "$a" "${c:0:7}" >> "$OUT"; continue
    fi
    v=$(grep -om1 'VERDICT: *\(OK\|SUSPECT\)' "$DIR/$a.txt" | sed 's/VERDICT: *//'); v=${v:-ERROR}
    note=$(grep -A6 -m1 'VERDICT' "$DIR/$a.txt" | tail -n +2 | grep -v '^\s*$' | head -3 | tr '\t\n' '  ' | cut -c1-300)
    printf '%s\t%s\t%s\t%s\n' "$a" "${c:0:7}" "$v" "$note" >> "$OUT"
  done
}

commits() {
  if [ $# -gt 0 ]; then printf '%s\n' "$@"; return; fi
  git log --format='%H %s' -40 | grep -E ' (feat: [0-9]* routines from .*round.*codewhale|codex round [0-9]+)' | cut -d' ' -f1 | tac
}

while :; do
  st=$(strata-local status 2>/dev/null); echo "$st" | grep -q ACTIVO || { echo "strata not running"; [ $WATCH = 1 ] && { sleep 300; continue; } || exit 1; }
  for c in $(commits "$@"); do review "$c"; done
  [ $WATCH = 1 ] || break
  sleep 300
done

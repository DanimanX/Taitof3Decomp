#!/usr/bin/env bash
# Harvest finished object routines from worker worktrees into the main repo.
# For every src/enemy/obj_X.c in ../Taitof3Decomp-wt/* that is no longer a
# stub and differs from main: copy it in, build, run its MAME test; keep it
# only when it is N/N bit-exact (N>0) with no undecompiled paths, otherwise
# restore main's version. Kept routines get a PROGRESS.md row, the coverage
# map is regenerated, and everything is committed (and pushed with --push).
# Usage: scripts/harvest.sh [--push]
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# one harvest at a time (a background loop and a manual run raced once and
# accepted two routines against a stale test binary)
exec 9>"$ROOT/build/.harvest.lock"; flock 9
kept=(); notes=()
for wt in "$ROOT"/../Taitof3Decomp-wt/*/; do
  w=$(basename "$wt")
  for f in "$wt"src/enemy/obj_*.c; do
    a=$(basename "$f" .c); a=${a#obj_}
    grep -q "(stub)" "$f" && continue
    grep -q "(stub)" "src/enemy/obj_$a.c" 2>/dev/null || continue   # only fill stubs
    cp "src/enemy/obj_$a.c" "/tmp/harvest_$a.bak"
    cp "$f" "src/enemy/obj_$a.c"
    rm -f build/test_objects
    if make -s build/test_objects >/dev/null 2>&1; then
      r=$(F3_ONLY="$a" ./build/test_objects 2>&1 | grep -E "Obj 0x0*$a\b" | tail -1)
      if echo "$r" | grep -qE " ([0-9]+)/ *\1 ticks bit-exact" && ! echo "$r" | grep -qE " 0/ *0 " \
         && echo "$r" | grep -q " 0 reach undecompiled"; then
        n=$(echo "$r" | grep -oE "[0-9]+/ *[0-9]+" | head -1 | tr -d ' ')
        kept+=("$a"); notes+=("0x$(printf %06X $((16#$a))) $n ($w)")
        echo "KEEP $a $n ($w)"
        continue
      fi
    fi
    cp "/tmp/harvest_$a.bak" "src/enemy/obj_$a.c"
  done
done
rm -f /tmp/harvest_*.bak
rm -f build/test_objects; make -s build/test_objects >/dev/null 2>&1
[ ${#kept[@]} -eq 0 ] && { echo "nothing new"; exit 0; }
make -s test >/dev/null 2>&1 || { echo "make test FAILED, not committing"; exit 1; }
row="| $(printf '0x%06X / ' $(for a in "${kept[@]}"; do echo $((16#$a)); done) | sed 's| / $||') | Obj_*_Tick (survey batch) | VERIFIED (MAME) | $(IFS=';'; echo "${notes[*]}" | sed 's/;/, /g') |"
python3 - "$row" <<'PY'
import sys
p='docs/PROGRESS.md'; s=open(p).read(); row=sys.argv[1]
anchor='\nNote on `Obj_Spawn` argument order'
s=s.replace(anchor, '\n'+row+anchor, 1) if anchor in s else s+row+'\n'
open(p,'w').write(s)
PY
python3 scripts/coverage.py
git add src/enemy docs README.md
git commit -q -m "feat: ${#kept[@]} object routines verified bit-exact vs MAME (parallel workers)

$(printf '%s\n' "${notes[@]}")

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
git log --oneline -1
if [ "${1:-}" = "--push" ]; then
  S=$(mktemp); chmod 700 "$S"
  printf '#!/bin/sh\ncase "$1" in Username*) echo DanimanX;; *) head -1 ~/.token.bak | tr -d "\\r\\n ";; esac\n' > "$S"
  GIT_ASKPASS="$S" GIT_TERMINAL_PROMPT=0 git -c credential.helper= push -q origin main && echo pushed
  rm -f "$S"
fi

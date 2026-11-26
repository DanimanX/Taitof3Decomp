#!/usr/bin/env bash
# agy_loop.sh FIRST_ROUND [FIRST_HI] : run agy rounds back to back with no
# Claude turn in between. Each round: agy_round.sh (30 routines, Hydrogen Medium
# unless AGY_MODEL is set) -> agy_merge.sh -> hold back anything jt_scan flags
# -> full tests -> commit + push -> clean up -> next round.
# The first round takes sizes 1..FIRST_HI (default 2000); later rounds take all.
#
# Stops (and prints why) when: the pool is empty, a round merges nothing,
# the build/tests fail (the round's merge is undone with git checkout/clean),
# or a push fails. An agy quota error is waited out (reset time parsed from the
# error, plus 3 min) and the same round is retried once.
#
# Routines with -1 returns are committed (they pass bit-exact and most are the
# usual Obj_AnimInit/ObjScript guards) but listed in work/zcap/review_queue.tsv
# for a later batched review. Log: work/zcap/agy_loop.log
# RESUME=1: the first pass only merges what is already in wt_G1..G3 (round
# FIRST_ROUND-1, still running or finished) after waiting for its agy processes.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=${1:?first round number}; FIRST_HI=${2:-2000}
LOG=work/zcap/agy_loop.log
say() { echo "$(date +%H:%M) $*" | tee -a "$LOG"; }

push() {
  local A; A=$(mktemp); printf '#!/bin/sh\ncat ~/.token.bak\n' > "$A"; chmod 700 "$A"
  GIT_ASKPASS=$A git push -q origin main 2>&1 | tail -2; local rc=${PIPESTATUS[0]}; rm -f "$A"; return $rc
}

cleanup() {
  # only things this pipeline creates
  rm -f build/fn1_* build/fn1_*.out
  for w in G1 G2 G3; do
    W=/PROCESOS/wt_$w; [ -d "$W" ] || continue
    rm -rf "$W/work/drafts" "$W"/work/fn_*_draft.c "$W"/build/fn1_* 2>/dev/null
  done
  ls -1t work/zcap/agy[0-9]*_G*.log 2>/dev/null | tail -n +31 | xargs -r gzip -f
}

hi=$FIRST_HI; retried=0
if [ "${RESUME:-0}" = 1 ]; then
  N=$((N-1)); say "resume: waiting for round $N's agy processes"
  while pgrep -f 'agy --model' >/dev/null; do sleep 60; done
fi
while :; do
  if [ "${RESUME:-0}" = 1 ]; then RESUME=0; say "merging round $N"
  else
    say "round $N (sizes 1..$hi, model ${AGY_MODEL:-gemini-4-hydrogen-medium})"
    out=$(scripts/agy_round.sh "$N" 1 "$hi" 2>&1); echo "$out" >> "$LOG"
    if echo "$out" | grep -q 'taking 0'; then say "pool empty, stopping"; break; fi
  fi
  q=$(grep -ho 'Resets in [0-9hms]*' work/zcap/agy${N}_G*.log 2>/dev/null | head -1)
  m=$(scripts/agy_merge.sh 2>&1); echo "$m" >> "$LOG"
  merged=$(echo "$m" | sed -n 's/^merged \([0-9]*\).*/\1/p'); merged=${merged:-0}
  # routines agy never got to (quota, time) become eligible again
  for a in $(cat work/zcap/agy${N}_G*.list 2>/dev/null | cut -f1); do
    [ -e "src/auto/fn_$a.c" ] || echo "$m" | grep -q "$a:" || printf '%s\t0\tfail\tagy round %s: not attempted\n' "$a" "$N" >> work/zcap/strata_results.tsv
  done
  # hold back routines whose jump tables have unmentioned handlers
  flagged=$(echo "$m" | sed -n '/^-- jump tables:/,/^fn:/p' | grep -oE '^[0-9A-F]{6}' | grep -v '^10C548$\|^110EC6$' | while read -r a; do git status --short "src/auto/fn_$a.c" | grep -q '^??' && echo "$a"; done)
  if [ -n "$flagged" ]; then
    python3 scripts/unmerge.py work/zcap/partial $flagged >> "$LOG"
    for a in $flagged; do printf '%s\t0\tfail\tjt_scan: jump-table handler not cited, held for review\n' "$a" >> work/zcap/strata_results.tsv; done
    merged=$((merged - $(echo $flagged | wc -w)))
  fi
  if [ "$merged" -le 0 ]; then
    if [ -n "$q" ] && [ $retried -eq 0 ]; then
      h=$(echo "$q" | grep -oE '[0-9]+h' | tr -d h); mi=$(echo "$q" | grep -oE '[0-9]+m' | tr -d m)
      s=$(( ${h:-0}*3600 + ${mi:-0}*60 + 180 )); say "agy quota hit ($q), waiting ${s}s then retrying round $N"
      sleep $s; retried=1; continue
    fi
    say "round $N merged nothing, stopping"; break
  fi
  retried=0
  make -s -j16 build/test_fn build/test_objects >> "$LOG" 2>&1
  t1=$(./build/test_fn | tail -1); nf=$(./build/test_objects | grep -ci fail)
  if [ "$t1" != "fn: PASS (0 failing calls)" ] || [ "$nf" != 0 ]; then
    say "TESTS FAIL after round $N ($t1, objects failing $nf): undoing the merge, stopping"
    git checkout -q src/auto/fn_auto.h tests/fn_auto.inc; git clean -qf src/auto; break
  fi
  new=$(git status --short src/auto | sed -n 's/^?? src\/auto\/fn_\(.*\)\.c$/\1/p' | tr '\n' ' ')
  for a in $new; do
    k=$(grep -c 'return -1' "src/auto/fn_$a.c"); [ "$k" -gt 0 ] && printf '%s\tround %s\t%s x return -1\n' "$a" "$N" "$k" >> work/zcap/review_queue.tsv
  done
  cov=$(make -s coverage | tail -1)
  held=$(echo "$m" | sed -n 's/^held //p')
  git add src/auto tests/fn_auto.inc docs/COVERAGE.md README.md
  git commit -q -F - <<EOF
feat: $merged routines from agy round $N (${AGY_MODEL:-gemini-4-hydrogen-medium})

$new
All bit-exact in test_fn (D0/D1/A0/A1 and all RAM), merged by
scripts/agy_loop.sh: audit_auto.sh + jt_scan.py checked, test_fn and
test_objects pass. Held back: ${held:-none}${flagged:+; jump-table flags: $flagged}.
Routines with -1 returns are queued in work/zcap/review_queue.tsv.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
EOF
  if ! push; then say "push failed after round $N (committed locally), stopping"; break; fi
  say "round $N: merged $merged, $cov, pushed $(git log --oneline -1 | cut -c1-7)"
  cleanup
  N=$((N+1)); [ "${FIRST_DONE:-0}" = 1 ] && hi=2000; FIRST_DONE=1
done
for w in G1 G2 G3; do git worktree remove --force /PROCESOS/wt_$w 2>/dev/null; done; git worktree prune
cleanup
say "loop finished at round $N"

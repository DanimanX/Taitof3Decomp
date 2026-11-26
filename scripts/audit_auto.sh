#!/usr/bin/env bash
# audit_auto.sh WORKTREE : checks every new src/auto routine in a worker worktree
# before merging: files outside src/auto, register check mode, -1 returns,
# raw Sound_PlayCmd calls, and whether an empty body would pass its capture.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; W=$1; cd "$ROOT"
echo "outside src/auto: $(git -C "$W" status --short | grep -v '^?? work' | grep -v 'src/auto/' | grep -v 'tests/fn_auto.inc' | tr '\n' ' ')"
for f in $(git -C "$W" status --short | awk '/^\?\? src\/auto\/fn_[0-9A-F]+\.c/{print $2}'); do
  a=$(basename "$f" .c); a=${a#fn_}
  mode=$(grep "fn_${a}\"" "$W/tests/fn_auto.inc" | grep -q ALLREGS && echo regs || echo ram-only)
  neg=$(grep -c 'return -1' "$W/$f"); raw=$(grep -c 'Sound_PlayCmd(' "$W/$f")
  printf '#include "fn_auto.h"\nint Fn_%s(const f3mem_t *m, f3regs_t *r){(void)m;(void)r;return 0;}\n' "$a" > "work/zcap/drafts/null_$a.c"
  cc -std=c11 -O1 -iquote src/auto -DFN1=Fn_$a -DFN1_ADDR="\"$a\"" -o "build/null_$a" tests/test_fn.c "work/zcap/drafts/null_$a.c" build/libf3.a 2>/dev/null
  nul=$(./build/null_$a | grep -c ' @')
  echo "$a $mode ret-1=$neg rawPlayCmd=$raw null_ramdiffs=$nul"
done

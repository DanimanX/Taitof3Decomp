#!/usr/bin/env bash
# Copy a codewhale worktree's new src/auto files, prototypes and test lines
# into the main tree, rebuild, run test_fn (timeout, capped output) and print
# the strength check (real vs empty Fn) of every new test entry.
# Usage: scripts/cw_integrate.sh [/PROCESOS/wt_CW]
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
W=${1:-/PROCESOS/wt_CW}
new=$(cd "$W" && git status --short src/auto | sed -n 's/^?? //p')
for f in $new; do [ -e "$f" ] && echo "exists $f" || cp "$W/$f" "$f"; done
# edits to existing shared src/auto headers/files (not the generated ones)
(cd "$W" && git diff -- src/auto ':!src/auto/fn_auto.h') > build/cw_mod.patch
[ -s build/cw_mod.patch ] && { git apply --3way build/cw_mod.patch || echo "cw_mod.patch did not apply"; }
(cd "$W" && git diff -U0 src/auto/fn_auto.h | grep '^+[^+]' | sed 's/^+//') > build/cw_protos.txt
(cd "$W" && git diff -U0 tests/fn_auto.inc | grep '^+[^+]' | sed 's/^+//') > build/cw_inc.txt
while IFS= read -r l; do grep -qF "$l" tests/fn_auto.inc || echo "$l" >> tests/fn_auto.inc; done < build/cw_inc.txt
python3 - <<'E'
import re, glob
p = 'src/auto/fn_auto.h'; h = open(p).read()
add = ''.join(l for l in open('build/cw_protos.txt') if l.strip() and l not in h)
# any Fn_<ADDR>(m, r) defined in src/auto without a prototype
for f in glob.glob('src/auto/fn_[0-9A-F]*.c'):
    a = f.split('fn_')[1][:-2]
    if f'Fn_{a}(' not in h + add and re.search(rf'^int Fn_{a}\(const f3mem_t \*m, f3regs_t \*r\)', open(f).read(), re.M):
        add += f'int Fn_{a}(const f3mem_t *m, f3regs_t *r); /* ROM 0x{a} */\n'
h = h.replace('/* END generated */', add + '/* END generated */', 1); open(p, 'w').write(h)
E
make -s build/test_fn build/libf3.a 2>&1 | head -5
timeout 1800 ./build/test_fn 2>&1 | grep -v '^f3mem' | head -c 20000000 | tail -1
for x in $(grep -oE '"work/ram/[a-z]+_[0-9A-F]{6}", Fn_[0-9A-F]{6}' build/cw_inc.txt | sed -E 's|"work/ram/([a-z]+_[0-9A-F]{6})", Fn_([0-9A-F]{6})|\2:\1|'); do
  scripts/check_capture.sh "${x%%:*}" "${x##*:}" &
done; wait
for f in $new; do head -4 "$f" | grep -q UNVERIFIED && echo "UNVERIFIED $f"; done

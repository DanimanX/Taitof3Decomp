#!/usr/bin/env bash
# One automatic wave: pick the smallest still-stubbed routines that have a
# disassembly bundle and no captures yet blocked on unknown helpers, prepare
# three worktrees, print the batch prompts. Verification is the parent's job
# (scripts/verify_set.sh + a commit), never the workers'.
#
# Why "no captures": a capture whose ticks all reach undecompiled paths gives
# the gate nothing to compare, so those routines cannot be accepted yet and are
# excluded (they need their callees first).
#
# Usage: scripts/next_wave.sh [count_per_worker=6] [wave_name]
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=${1:-6}
WAVE=${2:-wave$(( $(git worktree list | grep -c 'worktrees/wave' ) + 7 ))}
python3 - "$N" "$WAVE" <<'PY'
import glob, os, re, sys
n = int(sys.argv[1]); wave = sys.argv[2]
have = set()
for p in glob.glob('work/disasm/*.s'):
    m = re.search(r'(?:bundle_|_)([0-9A-F]{4,6})(?:_table)?\.s$', p)
    if m: have.add(int(m.group(1), 16))
dec = set()
for p in glob.glob('src/**/*.c', recursive=True):
    s = open(p).read()
    if 'not decompiled yet (stub)' in s: continue
    dec |= {int(x, 16) for x in re.findall(r'ROM 0x([0-9A-Fa-f]{4,6})', s)}
# already accepted (a ROM annotation in a non-stub file) or already tried and
# rejected: only routines whose source is still the shipped stub are candidates
unimpl = set()
for p in glob.glob('src/enemy/obj_*.c'):
    s2 = open(p).read()
    if 'not decompiled yet (stub)' not in s2:
        unimpl.add(int(os.path.basename(p)[4:-2], 16))
cand = []
for p in glob.glob('src/enemy/obj_*.c'):
    s2 = open(p).read()
    if 'not decompiled yet (stub)' not in s2: continue
    a = int(os.path.basename(p)[4:-2], 16)
    if a in unimpl: continue
    b = f'work/disasm/bundle_{a:05X}.s'
    if a not in have or not os.path.exists(b): continue
    js = re.findall(r'jsr 0x([0-9a-fA-F]{6})', open(b).read())
    if [t for t in js if int(t, 16) not in dec]: continue   # callees missing
    cand.append((os.path.getsize(b), a))
cand.sort()
pick = [a for _, a in cand[: n * 3]]
for i in range(3):
    print(f"{wave}-{chr(97 + i)}: {' '.join('%06X' % a for a in pick[i * n:(i + 1) * n])}")
PY

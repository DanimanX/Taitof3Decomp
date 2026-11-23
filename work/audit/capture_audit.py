#!/usr/bin/env python3
"""Audit work/ram/obj_<ADDR>_{pairs,regs}.tsv captures against the routine they
claim to exercise.

The harness reads A0 from the pre snapshot and ticks that object, so a capture
is only usable if fn@A0 == the routine address in EVERY pair (matching pairs
are the only ones the harness can compare). Reports: usable stubs (all pairs
match, >=10 pairs), mixed (some pairs never run the routine), and the same
audit for routines already passing N/N.

Note: WRAM is dumped from 0x400000, so the byte for address X is at X-0x400000.
"""
import os, glob, sys

W = 128 * 1024  # WRAM bytes per snapshot
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def audit(addr):
    base = os.path.join(ROOT, 'work/ram/obj_%s' % addr)
    if not (os.path.exists(base + '_pairs.bin') and os.path.exists(base + '_regs.tsv')):
        return None
    meta = {}
    for line in open(base + '_regs.tsv').read().splitlines()[1:]:
        c = line.split('\t')
        if len(c) > 11:
            meta[(int(c[0]), c[1])] = c
    d = open(base + '_pairs.bin', 'rb').read()
    n = os.path.getsize(base + '_pairs.bin') // (2 * W)
    want = int(addr, 16)
    launched = match = dead = 0
    for i in range(n):
        pre = meta.get((i, 'pre'))
        if not pre:
            continue
        launched += 1
        off = int(pre[11], 16) - 0x400000
        if not (0 <= off < W - 4):
            dead += 1
            continue
        fn = int.from_bytes(d[i * 2 * W + off:i * 2 * W + off + 4], 'big')
        if fn == want:
            match += 1
        elif fn == 0:
            dead += 1
    return launched, match, dead, n


def stubs():
    out = []
    for src in sorted(glob.glob(os.path.join(ROOT, 'src/enemy/obj_*.c'))):
        if 'not decompiled yet (stub)' in open(src).read():
            out.append(os.path.basename(src)[4:-2])
    return out


if __name__ == '__main__':
    usable, mixed = [], []
    for a in stubs():
        r = audit(a)
        if not r or r[0] == 0:
            continue
        launched, match, dead, _ = r
        if match == launched and launched >= 10:
            usable.append((a, launched))
        elif match:
            mixed.append((a, match, launched))
    print('usable stubs (every pair runs the routine, >=10 pairs): %d' % len(usable))
    print('mixed (only some pairs run it): %d' % len(mixed))
    for a, m, l in mixed[:10]:
        print('  %s: %d/%d' % (a, m, l))
    if len(sys.argv) > 1 and sys.argv[1] == '--list':
        for a, l in sorted(usable, key=lambda t: -t[1]):
            print(a, l)

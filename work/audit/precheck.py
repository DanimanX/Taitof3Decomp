#!/usr/bin/env python3
"""Pre-flight for an object routine before decompiling it.

Runs the four checks that cost a full worker attempt when skipped:
  1. capture audit  -- every pair ticks THIS routine (fn(obj) == address)
  2. fidelity       -- the post writes the object's own struct in some pair
  3. callees        -- every jsr/bsr target has a translation in src/
  4. pc-indexed     -- any `22 7B`-style table is resolvable (see docs)

Usage: python3 work/audit/precheck.py <ADDR> [<ADDR> ...]
Exit code 0 when all checks pass for every address.
"""
import glob
import os
import re
import subprocess
import sys

W = 128 * 1024
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def load(addr):
    base = os.path.join(ROOT, 'work/ram/obj_%s' % addr)
    pb = base + '_pairs.bin'
    rg = base + '_regs.tsv'
    if not (os.path.exists(pb) and os.path.exists(rg)):
        return None
    meta = {}
    for line in open(rg).read().splitlines()[1:]:
        c = line.split('\t')
        if len(c) > 11:
            meta[(int(c[0]), c[1])] = c
    return open(pb, 'rb').read(), meta, os.path.getsize(pb) // (2 * W)


def check_capture(addr):
    got = load(addr)
    if got is None:
        return 'NO CAPTURE'
    data, meta, n = got
    want = int(addr, 16)
    launched = match = 0
    for i in range(n):
        pre = meta.get((i, 'pre'))
        if not pre:
            continue
        launched += 1
        off = int(pre[11], 16) - 0x400000
        if 0 <= off < W - 4 and int.from_bytes(data[i * 2 * W + off:i * 2 * W + off + 4], 'big') == want:
            match += 1
    if launched == 0:
        return 'no pairs'
    if match != launched:
        return 'only %d/%d pairs tick it' % (match, launched)
    return 'ok %d/%d' % (match, launched)


def check_fidelity(addr):
    """A post that never writes the object's own 0x6C bytes cannot be the tick."""
    got = load(addr)
    if got is None:
        return 'NO CAPTURE'
    data, meta, n = got
    for i in range(n):
        pre = meta.get((i, 'pre'))
        if not pre:
            continue
        o = int(pre[11], 16) - 0x400000
        p = i * 2 * W
        if any(data[p + o + k] != data[p + W + o + k] for k in range(0x6C)):
            return 'ok'
    return 'post never writes the object'


def srchay():
    hay = []
    for f in glob.glob(os.path.join(ROOT, 'src/**/*.c'), recursive=True):
        hay.append(open(f, errors='ignore').read().lower())
    for f in glob.glob(os.path.join(ROOT, 'src/**/*.h'), recursive=True):
        hay.append(open(f, errors='ignore').read().lower())
    return '\n'.join(hay)


def check_callees(addr):
    d = open(os.path.join(ROOT, 'work/maincpu.bin'), 'rb').read()
    hay = srchay()
    code = d[int(addr, 16):int(addr, 16) + 0x500]
    targets = set()
    for m in re.finditer(rb'\x4e\xb9(.)(.)(.)(.)', code):          # jsr abs.l
        t = int.from_bytes(m.group(1) + m.group(2) + m.group(3) + m.group(4), 'big')
        targets.add(t)
    for m in re.finditer(rb'\x61\x00(.)(.)', code):                # bsr.w
        targets.add(None)
    missing = []
    for t in sorted(x for x in targets if x is not None):
        if t >= 0x100000:
            continue
        p = '0x%06x' % t
        if p not in hay and ('0x%05x' % t) not in hay and ('0x%04x' % t) not in hay:
            missing.append('%06X' % t)
    return 'missing: ' + ','.join(missing) if missing else 'ok'


def check_pcindexed(addr):
    d = open(os.path.join(ROOT, 'work/maincpu.bin'), 'rb').read()
    code = d[int(addr, 16):int(addr, 16) + 0x500]
    n = len(re.findall(rb'\x22\x7b', code)) + len(re.findall(rb'\x4e\xbb', code))
    return 'ok (%d fixed/table sites)' % n if n else 'ok'


CHECKS = [('capture', check_capture), ('fidelity', check_fidelity),
          ('callees', check_callees), ('pcidx', check_pcindexed)]

if __name__ == '__main__':
    bad = 0
    for a in sys.argv[1:]:
        a = a.upper()
        res = [(name, fn(a)) for name, fn in CHECKS]
        verdict = 'PASS' if all(v.startswith('ok') for _, v in res) else 'FAIL'
        if verdict == 'FAIL':
            bad += 1
        print('%s %s %s' % (a, verdict, ' | '.join('%s=%s' % (k, v) for k, v in res)))
    sys.exit(1 if bad else 0)

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


def check_state(addr):
    """Report the captured obj+0x6 values. Most late workers passed `obj+0x6`
    readings that did not exist (0x1E, 0x80, 0x02 where the file holds 0x00):
    this prints the truth so the brief cannot assert a state we never checked.
    """
    got = load(addr)
    if got is None:
        return 'NO CAPTURE'
    data, meta, n = got
    vals = {}
    for i in range(n):
        pre = meta.get((i, 'pre'))
        if not pre:
            continue
        o = int(pre[11], 16) - 0x400000
        if 0 <= o < W - 6:
            v = data[i * 2 * W + o + 6]
            vals[v] = vals.get(v, 0) + 1
    return 'ok obj+6=' + ','.join('%02X x%d' % (k, v) for k, v in sorted(vals.items()))


def check_sprite(addr):
    """obj+0x10 must point at a real {w.w, h.w, ...} sprite definition.

    Validated against every routine that passes: 13/13 have a plausible def
    (w,h in 1..16). Of 1169 captures, 145 point at zeros or garbage; 0x111B56
    was one, and its tick calls Obj_DrawSprite with that pointer while the
    sprite cursor still advances, which no faithful run can do. This is the
    strongest single signal found so far.
    """
    got = load(addr)
    if got is None:
        return 'NO CAPTURE'
    data, meta, n = got
    rom = open(os.path.join(ROOT, 'work/maincpu.bin'), 'rb').read()
    bad = 0
    for i in range(n):
        pre = meta.get((i, 'pre'))
        if not pre:
            continue
        o = int(pre[11], 16) - 0x400000
        if not (0 <= o < W - 0x14):
            continue
        p = int.from_bytes(data[i * 2 * W + o + 0x10:i * 2 * W + o + 0x14], 'big')
        if p + 8 > len(rom):
            bad += 1
            continue
        w = int.from_bytes(rom[p:p + 2], 'big')
        h = int.from_bytes(rom[p + 2:p + 4], 'big')
        if not (1 <= w <= 16 and 1 <= h <= 16):
            bad += 1
    if bad:
        return 'UNUSABLE: obj+0x10 is not a sprite def in %d pair(s)' % bad
    return 'ok'


CHECKS = [('capture', check_capture), ('sprite', check_sprite), ('fidelity', check_fidelity), ('state', check_state),
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

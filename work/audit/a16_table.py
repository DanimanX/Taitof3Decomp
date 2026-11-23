#!/usr/bin/env python3
"""Resolve the table base of a 68020 PC-indexed `movea.l (bd,PC,Xn),An`.

RULE, verified against a known-good case: the base is the address of the
EXTENSION word plus bd. For `22 7B` at P the extension word is at P+2, so
base = (P+2) + bd. Cross-check that gave the right answer for the jump table
of obj_107112: `4E BB 05 21 005A` at 0x1071B6 -> base 0x107212, whose four
entries are 0x107222/0x10725E/0x107378/0x1073A6 (the handlers that routine
already uses).

Usage: python3 work/audit/a16_table.py <instr_hex>
Prints candidates; pick the one whose entries are all ROM pointers AND whose
records decode as {w, w, l} through the caller's own stride (usually 8).
"""
import sys

d = open('work/maincpu.bin', 'rb').read()


def entries(base, n=4, stride=4):
    return [int.from_bytes(d[base + i * stride:base + i * stride + 4], 'big') for i in range(n)]


def main():
    p = int(sys.argv[1], 16)
    ext2 = int.from_bytes(d[p + 2:p + 4], 'big')
    ext3 = int.from_bytes(d[p + 4:p + 6], 'big')
    is_full = (ext2 >> 14) & 1
    # BD is SIGNED: 0xFD40 must be read as -0x2C0 or the base lands in the
    # wrong bank (that is exactly how this tool first mis-ranked 0x107CD0).
    if is_full:
        if (ext2 >> 8) & 1:
            bd = int.from_bytes(d[p + 6:p + 8], 'big', signed=True)
        else:
            bd = int.from_bytes(d[p + 6:p + 7], 'big', signed=True)
    else:
        bd = int.from_bytes(d[p + 4:p + 6], 'big', signed=True)
    print('instruction %06X  ext=%04X ext2=%04X  bd=%d' % (p, ext2, ext3, bd))
    base = p + 2 + bd
    e = entries(base)
    # A NULL entry is legitimate: those modes have no handler in the ROM.
    ok = all(v == 0 or 0x400 <= v < 0x200000 for v in e) and any(e)
    print('  base (P+2+bd) = %06X  %s  %s' % (base, [hex(v) for v in e],
                                             'POINTERS (0 = NULL entry)' if ok else 'not a table'))
    if ok:
        for i, v in enumerate(e):
            print('    [%d] %06X -> %s' % (i, v, d[v:v + 8].hex(' ')))


if __name__ == '__main__':
    main()

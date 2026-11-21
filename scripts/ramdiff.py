#!/usr/bin/env python3
"""Find Work RAM words that behave like player X/Y or score across the
scripted session dumps (scripts/lua/session_ramdiff.lua -> work/ram/*.bin).

X: idle->right increases, right->left decreases, unchanged-ish by up/down.
Y: up/down phases move it in opposite directions, left/right don't.
Score: BCD-looking value that increases during the fire phase.
"""
import struct
import sys

D = "work/ram/"
names = ["10_idle", "11_after_right", "12_after_left", "13_after_up", "14_after_down", "15_after_fire"]
ram = {n: open(D + n + ".bin", "rb").read() for n in names}
BASE = 0x400000


def words(n, signed=False):
    fmt = ">%dh" % (len(ram[n]) // 2) if signed else ">%dH" % (len(ram[n]) // 2)
    return struct.unpack(fmt, ram[n])


w = {n: words(n, True) for n in names}
print("== X candidates (word): right up, left down, up/down ~0 ==")
for i in range(len(w[names[0]])):
    i0, r, l, u, d = (w[n][i] for n in names[:5])
    dr, dl, du, dd = r - i0, l - r, u - l, d - u
    if dr > 8 and dl < -8 and abs(du) <= 2 and abs(dd) <= 2:
        print("  %06X idle=%d right=%d left=%d up=%d down=%d" % (BASE + 2 * i, i0, r, l, u, d))
print("== Y candidates (word): up/down opposite, left/right ~0 ==")
for i in range(len(w[names[0]])):
    i0, r, l, u, d = (w[n][i] for n in names[:5])
    dr, dl, du, dd = r - i0, l - r, u - l, d - u
    if abs(dr) <= 2 and abs(dl) <= 2 and du * dd < 0 and abs(du) > 8 and abs(dd) > 8:
        print("  %06X idle=%d right=%d left=%d up=%d down=%d" % (BASE + 2 * i, i0, r, l, u, d))
print("== score candidates (long, BCD, grows in fire phase) ==")
def bcd(v):
    return all(((v >> s) & 0xF) <= 9 for s in range(0, 32, 4))
a = ram["14_after_down"]; b = ram["15_after_fire"]
for off in range(0, len(a), 2):
    x = struct.unpack(">I", a[off:off + 4])[0] if off + 4 <= len(a) else 0
    y = struct.unpack(">I", b[off:off + 4])[0] if off + 4 <= len(b) else 0
    if y > x and bcd(x) and bcd(y) and y - x < 0x01000000 and (y & 0xFF) == 0:
        print("  %06X before=%08X after=%08X" % (BASE + off, x, y))

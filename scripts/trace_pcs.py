#!/usr/bin/env python3
"""Read a MAME maincpu trace (stdin or FIFO path) and write
<out>.pcs (unique executed PCs) and <out>.calls (targets reached right after
a jsr/bsr: function entries; jmp targets go to <out>.jmps).
Usage: scripts/trace_pcs.py <fifo> <out_prefix>"""
import sys, re
src, out = sys.argv[1], sys.argv[2]
pcs, calls, jmps = set(), set(), set()
prev = ""
rx = re.compile(r"^([0-9A-Fa-f]{6,8}): (\S+)")
with open(src, "r", errors="replace") as f:
    for ln in f:
        m = rx.match(ln)
        if not m:
            continue
        pc = int(m.group(1), 16) & 0xFFFFFF
        if pc < 0x200000:
            pcs.add(pc)
            if prev.startswith(("jsr", "bsr")):
                calls.add(pc)
            elif prev.startswith("jmp"):
                jmps.add(pc)
        prev = m.group(2)
for ext, s in (("pcs", pcs), ("calls", calls), ("jmps", jmps)):
    with open(f"{out}.{ext}", "w") as g:
        g.write("".join("%06X\n" % a for a in sorted(s)))
print(f"trace_pcs: {len(pcs)} PCs, {len(calls)} call targets, {len(jmps)} jmp targets")

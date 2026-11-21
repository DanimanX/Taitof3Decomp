#!/usr/bin/env python3
"""List task-spawn sites in a Taito F3 maincpu image (dariusg kernel).

Matches `jsr Sys_Spawn.w` (4EB8 1E60), `jsr Sys_Spawn.l` (4EB9 0000 1E60) and
`trap #1` (4E41), then looks back up to 24 bytes for `pea abs.l` (4879 xxxxxxxx)
or `pea (d16,PC)` (487A dddd) giving the task entry point.
Usage: scripts/find_spawns.py [work/maincpu.bin]
"""
import struct
import sys

d = open(sys.argv[1] if len(sys.argv) > 1 else "work/maincpu.bin", "rb").read()
PATS = {b"\x4e\xb8\x1e\x60": "jsr.w", b"\x4e\xb9\x00\x00\x1e\x60": "jsr.l", b"\x4e\x41": "trap1"}
rows = []
for pat, kind in PATS.items():
    i = 0
    while (i := d.find(pat, i)) >= 0:
        if i % 2 == 0:
            entry = None
            for back in range(2, 26, 2):
                j = i - back
                if d[j:j + 2] == b"\x48\x79":
                    entry = struct.unpack(">I", d[j + 2:j + 6])[0]
                    break
                if d[j:j + 2] == b"\x48\x7a":
                    entry = j + 2 + struct.unpack(">h", d[j + 2:j + 4])[0]
                    break
            rows.append((i, kind, entry))
        i += 2
for i, kind, entry in sorted(rows):
    print("%06X  %-6s entry=%s" % (i, kind, "%06X" % entry if entry is not None else "?"))
print("# %d sites, %d distinct entries" % (len(rows), len({e for _, _, e in rows if e})))

#!/usr/bin/env python3
"""Flag src/auto routines whose disassembly bundle has a jump table with
handler addresses the C file never mentions (possible untranslated
entries). Tables are dumped from work/maincpu.bin until an entry stops
looking like a code address near the others. Heuristic: a C file that
cites the handler address in a comment or constant counts as covering it."""
import glob, os, re, struct
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
rom = open("work/maincpu.bin", "rb").read()
SHARED = {0x0D4F26}  # 0x0D1EBC multiply table, verified equivalent
for c in sorted(glob.glob("src/auto/fn_*.c")):
    a = os.path.basename(c)[3:-2]
    bp = f"work/disasm/bundle_{a.lstrip('0')}.s"
    if not os.path.exists(bp): continue
    src = open(c).read().upper()
    for t in {int(x, 16) for x in re.findall(r"jump table ([0-9A-Fa-f]+)", open(bp).read())}:
        if t in SHARED: continue
        ents = []
        for i in range(64):
            v = struct.unpack(">I", rom[t + 4 * i:t + 4 * i + 4])[0]
            if v == 0 and i == 0: ents.append(0); continue
            if not (0x400 <= v < 0x200000) or (ents and [e for e in ents if e] and abs(v - max(e for e in ents if e)) > 0x4000): break
            ents.append(v)
        miss = sorted({v for v in ents if v and f"{v:06X}" not in src and f"{v:X}" not in src})
        if miss:
            print(f"{a}\ttable {t:06X} ({len(ents)} entries)\tunmentioned: {' '.join(f'{v:06X}' for v in miss)}")

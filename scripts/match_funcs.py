#!/usr/bin/env python3
"""Match the functions of another Taito F3 game against dariusg's.

Both function lists come from Ghidra (work/functions.tsv and
<game>/work/functions.tsv). A function is disassembled linearly from its
entry (capstone, 68020) for its instruction-byte size, and each instruction
becomes a normalised token: mnemonic + operands with every number of 3+ hex
digits (absolute addresses, A5 displacements, big immediates) and every
branch/call target replaced by a placeholder. Two functions match when their
token lists are identical; small constants and register use must agree.

Output <game>/work/match_dariusg.tsv:
  game_addr  size  dariusg_addr  kind  name
kind: exact (same bytes), reloc (same tokens, only masked numbers differ),
      ambiguous (several dariusg functions share the tokens: first listed).
Usage: scripts/match_funcs.py <game> [min_size=8]
"""
import sys, re, os, capstone
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
game = sys.argv[1]
minsz = int(sys.argv[2]) if len(sys.argv) > 2 else 8
md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN | capstone.CS_MODE_M68K_020)
BIG = re.compile(r"\$[0-9a-fA-F]{3,}|0x[0-9a-fA-F]{3,}")
BRANCH = re.compile(r"^(b[a-z]{2}|bra|bsr|db[a-z]{1,2}|jsr|jmp)(\.[bwsl])?$")


def load(tsv):
    fs = {}
    for ln in open(tsv):
        p = ln.rstrip("\n").split("\t")
        if p[0].startswith("#"):
            continue
        fs[int(p[0], 16)] = (int(p[1]), p[2])
    return fs


def tokens(rom, a, n):
    out, end = [], a + n
    for i in md.disasm(rom[a:end + 16], a):
        if i.address >= end:
            break
        ops = i.op_str
        if BRANCH.match(i.mnemonic):
            ops = "T"
        else:
            ops = BIG.sub("N", ops)
        out.append(i.mnemonic + " " + ops)
    return tuple(out)


drom = open(f"{ROOT}/work/maincpu.bin", "rb").read()
grom = open(f"{ROOT}/{game}/work/maincpu.bin", "rb").read()
dfs = load(f"{ROOT}/work/functions.tsv")
gfs = load(f"{ROOT}/{game}/work/functions.tsv")
index = {}
for a, (n, name) in dfs.items():
    if n < minsz:
        continue
    index.setdefault((n, tokens(drom, a, n)), []).append(a)
rows, stats = [], {"exact": 0, "reloc": 0, "ambiguous": 0}
sizes = {k: 0 for k in stats}
for a, (n, name) in sorted(gfs.items()):
    if n < minsz:
        continue
    hit = index.get((n, tokens(grom, a, n)))
    if not hit:
        continue
    exact = [d for d in hit if drom[d:d + n] == grom[a:a + n]]
    kind = "exact" if exact else ("reloc" if len(hit) == 1 else "ambiguous")
    d = (exact or hit)[0]
    stats[kind] += 1
    sizes[kind] += n
    rows.append(f"{a:06X}\t{n}\t{d:06X}\t{kind}\t{dfs[d][1]}")
out = f"{ROOT}/{game}/work/match_dariusg.tsv"
with open(out, "w") as f:
    f.write("# game_addr\tsize\tdariusg_addr\tkind\tdariusg_name\n" + "\n".join(rows) + "\n")
gtot = sum(n for n, _ in gfs.values())
print(f"{game}: {len(gfs)} functions, {gtot} code bytes; matched "
      + ", ".join(f"{k} {stats[k]} ({sizes[k]} B)" for k in stats)
      + f" -> {sum(sizes.values()) / gtot:.1%} of code bytes")

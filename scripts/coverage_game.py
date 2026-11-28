#!/usr/bin/env python3
"""Decompilation coverage for a second Taito F3 game kept under <game>/.

Same metric as scripts/coverage.py (dariusg): instruction bytes of Ghidra
functions (<game>/work/functions.tsv) whose entry is annotated "ROM 0x..." in
<game>/src, over the instruction bytes of all functions. Verified: the
routine has a passing differential test entry in <game>/tests/fn_auto.inc
whose capture exists, or its C ran inside the passing suite
(<game>/work/gcov_exec.tsv), and its header does not say UNVERIFIED.
Writes <game>/docs/COVERAGE.md and the README block between
<!-- coverage:<game>:start --> and <!-- coverage:<game>:end -->.
Usage: scripts/coverage_game.py <game> "<Title>"
"""
import glob, re, sys, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
game, title = sys.argv[1], sys.argv[2]
G = f"{ROOT}/{game}"
funcs, total = {}, 0
for ln in open(f"{G}/work/functions.tsv"):
    p = ln.rstrip("\n").split("\t")
    if p[0] == "#total_insn_bytes":
        total = int(p[1])
    elif not p[0].startswith("#"):
        funcs[int(p[0], 16)] = (int(p[1]), p[2])
nc = f"{G}/work/not_code.tsv"
if os.path.exists(nc):
    for ln in open(nc):
        if ln.strip() and not ln.startswith("#"):
            funcs.pop(int(ln.split()[0], 16), None)
dec, unver = set(), set()
for f in glob.glob(f"{G}/src/**/*.[ch]", recursive=True):
    txt = open(f).read()
    for lst in re.findall(r"ROM (0x[0-9A-Fa-f]{4,6}(?:,\s*0x[0-9A-Fa-f]{4,6})*)", txt):
        dec |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", lst)}
    m = re.search(r"ROM 0x([0-9A-Fa-f]{4,6}): UNVERIFIED", "\n".join(txt.split("\n")[:8]))
    if m:
        unver.add(int(m.group(1), 16))
ver = set()
inc = f"{G}/tests/fn_auto.inc"
if os.path.exists(inc):
    for name, pfx in re.findall(r'"([^"]*0x[0-9A-Fa-f]{4,6}[^"]*)",\s*"([^"]+)"', open(inc).read()):
        if any(os.path.exists(f"{ROOT}/{pfx}_{s}") for s in ("pairs.bin", "regs.tsv")):
            ver |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", name)}
gx = f"{G}/work/gcov_exec.tsv"
if os.path.exists(gx):
    ver |= {int(ln.split()[0], 16) for ln in open(gx) if ln.strip() and not ln.startswith("#")}
ver = (ver & dec) - unver
dec_f = {a for a in funcs if a in dec}
ver_f = {a for a in dec_f if a in ver}
fsum = lambda s: sum(funcs[a][0] for a in s)
in_funcs = fsum(funcs)
pct = lambda x, y: 100.0 * x / y if y else 0.0
bar = lambda frac, w=20: "█" * round(frac * w) + "░" * (w - round(frac * w))
D, V = fsum(dec_f), fsum(ver_f)
summary = [
    "| | functions | code bytes | % of code in functions |",
    "|---|---:|---:|---:|",
    f"| Decompiled to C | {len(dec_f)} / {len(funcs)} | {D:,} / {in_funcs:,} | **{pct(D, in_funcs):.1f}%** |",
    f"| …of which verified bit-exact vs MAME | {len(ver_f)} | {V:,} | {pct(V, in_funcs):.1f}% |",
    f"| **Remaining** | {len(funcs) - len(dec_f)} | {in_funcs - D:,} | **{100 - pct(D, in_funcs):.1f}%** |",
]
overall = (f"**{pct(D, in_funcs):.1f}% decompiled · {100 - pct(D, in_funcs):.1f}% remaining**  \n"
           f"`{bar(D / in_funcs if in_funcs else 0, 40)}`")
rows = ["| ROM bank | functions (done/total) | code bytes | decompiled | map |", "|---|---:|---:|---:|---|"]
for b in range(0, 0x200000, 0x10000):
    fs = [a for a in funcs if b <= a < b + 0x10000]
    tb = fsum(fs)
    if tb:
        d = fsum(a for a in fs if a in dec_f)
        rows.append(f"| `0x{b:06X}` | {sum(1 for a in fs if a in dec_f)}/{len(fs)} | {tb:,} | "
                    f"{pct(d, tb):5.1f}% | `{bar(d / tb)}` |")
note = (f"Code bytes = instruction bytes of the {len(funcs)} Ghidra functions in maincpu "
        f"({in_funcs:,} of {total:,} disassembled bytes). The function list comes from Ghidra "
        f"seeded with every PC MAME executed in traced play (`{game}/work/functions.tsv`). "
        f"A function counts as decompiled when its entry is annotated `ROM 0x…` in `{game}/src/`. "
        f"Regenerate with `make coverage-{game}`.")
block = "\n".join([f"### Decompilation progress — {title} (`{game}`)", "", overall, "", *summary, "",
                   f"Per-bank map: [`{game}/docs/COVERAGE.md`]({game}/docs/COVERAGE.md). {note}"])
os.makedirs(f"{G}/docs", exist_ok=True)
open(f"{G}/docs/COVERAGE.md", "w").write("\n".join(
    [f"# Decompilation coverage — {title} (maincpu)", "", overall, "", *summary, "",
     "## Map by 64 KiB ROM bank", "", "`█` decompiled · `░` remaining (by code bytes)", "",
     *rows, "", note, ""]))
p = f"{ROOT}/README.md"
s = open(p).read()
S, E = f"<!-- coverage:{game}:start -->", f"<!-- coverage:{game}:end -->"
if S not in s:
    print(f"README.md has no {S} marker", file=sys.stderr)
    sys.exit(1)
s = s[:s.index(S) + len(S)] + "\n" + block + "\n" + s[s.index(E):]
open(p, "w").write(s)
print(f"{game}: decompiled {pct(D, in_funcs):.1f}% ({len(dec_f)}/{len(funcs)} functions), "
      f"verified {pct(V, in_funcs):.1f}%")

#!/usr/bin/env python3
"""Decompilation coverage map for Darius Gaiden (maincpu).

Inputs: work/functions.tsv (scripts/ghidra/export_functions.py), the
"ROM 0x......" annotations in src/ (decompiled routines, incl. static
helpers) and the VERIFIED rows of docs/PROGRESS.md.
Writes docs/COVERAGE.md and the block between the coverage markers in
README.md. Metric: instruction bytes of Ghidra functions whose entry point
is decompiled, over all disassembled instruction bytes in ROM.
"""
import glob, re, sys, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
funcs, total = {}, 0
for ln in open(f"{ROOT}/work/functions.tsv"):
    p = ln.rstrip("\n").split("\t")
    if p[0] == "#total_insn_bytes":
        total = int(p[1])
    else:
        funcs[int(p[0], 16)] = (int(p[1]), p[2])
dec = set()
for f in glob.glob(f"{ROOT}/src/**/*.[ch]", recursive=True):
    dec |= {int(x, 16) for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", open(f).read())}
# stubs (integrated so workers can fill them) are not decompiled
for f in glob.glob(f"{ROOT}/src/enemy/obj_*.c"):
    if "(stub)" in open(f).read():
        dec -= {int(x, 16) for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", open(f).read())}
        dec -= {int(os.path.basename(f)[4:-2], 16)}
ver = set()
for ln in open(f"{ROOT}/docs/PROGRESS.md"):
    if "VERIFIED" in ln and ln.startswith("|"):
        cell = ln.split("|")[1]
        ver |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", cell)}
dec_f = {a for a in funcs if a in dec}
ver_f = {a for a in dec_f if a in ver}
fsum = lambda s: sum(funcs[a][0] for a in s)
in_funcs = fsum(funcs)
BANK = 0x10000
def bar(frac, w=20):
    n = round(frac * w)
    return "█" * n + "░" * (w - n)
rows = []
for b in range(0, 0x200000, BANK):
    fs = [a for a in funcs if b <= a < b + BANK]
    tot_b = fsum(fs)
    if tot_b == 0:
        continue
    d = fsum(a for a in fs if a in dec_f)
    v = fsum(a for a in fs if a in ver_f)
    rows.append((b, len(fs), sum(1 for a in fs if a in dec_f), tot_b, d, v))
pct = lambda x, y: 100.0 * x / y if y else 0.0
D, V = fsum(dec_f), fsum(ver_f)
summary = [
    f"| | functions | code bytes | % of code in functions |",
    f"|---|---:|---:|---:|",
    f"| Decompiled to C | {len(dec_f)} / {len(funcs)} | {D:,} / {in_funcs:,} | **{pct(D, in_funcs):.1f}%** |",
    f"| …of which verified bit-exact vs MAME | {len(ver_f)} | {V:,} | {pct(V, in_funcs):.1f}% |",
    f"| **Remaining** | {len(funcs) - len(dec_f)} | {in_funcs - D:,} | **{100 - pct(D, in_funcs):.1f}%** |",
]
overall = (f"**{pct(D, in_funcs):.1f}% decompiled · {100 - pct(D, in_funcs):.1f}% remaining**  \n"
           f"`{bar(D / in_funcs, 40)}`")
bank_tbl = ["| ROM bank | functions (done/total) | code bytes | decompiled | map |",
            "|---|---:|---:|---:|---|"]
for b, nf, nd, tb, d, v in rows:
    bank_tbl.append(f"| `0x{b:06X}` | {nd}/{nf} | {tb:,} | {pct(d, tb):5.1f}% | `{bar(d / tb)}` |")
note = (f"Code bytes = instruction bytes of the {len(funcs)} Ghidra functions in maincpu "
        f"({in_funcs:,} of {total:,} disassembled bytes; the rest is code Ghidra has not "
        "assigned to a function yet). A function counts as decompiled when its entry "
        "address is annotated `ROM 0x…` in `src/`. Regenerate with `make coverage`.")
readme_block = "\n".join(["### Decompilation progress — Darius Gaiden (`dariusg`)", "", overall, "",
                          *summary, "", f"Per-bank map: [`docs/COVERAGE.md`](docs/COVERAGE.md). {note}"])
cov = "\n".join(["# Decompilation coverage — Darius Gaiden (maincpu)", "", overall, "", *summary, "",
                 "## Map by 64 KiB ROM bank", "", "`█` decompiled · `░` remaining (by code bytes)", "",
                 *bank_tbl, "", note, ""])
open(f"{ROOT}/docs/COVERAGE.md", "w").write(cov)
p = f"{ROOT}/README.md"; s = open(p).read()
S, E = "<!-- coverage:start -->", "<!-- coverage:end -->"
if S in s:
    s = s[:s.index(S) + len(S)] + "\n" + readme_block + "\n" + s[s.index(E):]
else:
    print("README.md has no coverage markers", file=sys.stderr); sys.exit(1)
open(p, "w").write(s)
print(f"decompiled {pct(D, in_funcs):.1f}% ({len(dec_f)}/{len(funcs)} functions), verified {pct(V, in_funcs):.1f}%")

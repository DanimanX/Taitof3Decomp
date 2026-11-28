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
# entries of functions.tsv that are data, not code (work/not_code.tsv)
for ln in open(f"{ROOT}/work/not_code.tsv"):
    if ln.strip() and not ln.startswith("#"):
        funcs.pop(int(ln.split()[0], 16), None)
dec = set()
for f in glob.glob(f"{ROOT}/src/**/*.[ch]", recursive=True):
    # "ROM 0xA" and comma lists "ROM 0xA, 0xB, 0xC" (one comment naming
    # several routines translated together)
    for lst in re.findall(r"ROM (0x[0-9A-Fa-f]{4,6}(?:,\s*0x[0-9A-Fa-f]{4,6})*)", open(f).read()):
        dec |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", lst)}
# stubs (integrated so workers can fill them) are not decompiled. Only the
# stub's own entry point is dropped: subtracting every "ROM 0x..." the stub
# file mentions also removed helpers that other, decompiled routines reuse.
for f in glob.glob(f"{ROOT}/src/enemy/obj_*.c"):
    if "(stub)" in open(f).read():
        dec -= {int(os.path.basename(f)[4:-2], 16)}
ver = set()
for ln in open(f"{ROOT}/docs/PROGRESS.md"):
    if "VERIFIED" in ln and ln.startswith("|"):
        cell = ln.split("|")[1]
        ver |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", cell)}
# Routines with their own differential test against a MAME capture on disk
# (test_fn: tests/fn_auto.inc + tests/test_fn.c; test_objects: run(...)), the
# suites being required to pass before every commit. Excluded: files tagged
# UNVERIFIED (weak or no capture) and rows of work/zcap/unverified.tsv.
def has_cap(pfx):
    return any(os.path.exists(f"{ROOT}/{pfx}_{s}") for s in ("pairs.bin", "regs.tsv"))
unver = set()
for ln in open(f"{ROOT}/work/zcap/unverified.tsv"):
    if ln.strip():
        unver.add(int(ln.split()[0], 16))
for f in glob.glob(f"{ROOT}/src/**/*.c", recursive=True):
    head = "\n".join(open(f).read().split("\n")[:6])
    m = re.search(r"ROM 0x([0-9A-Fa-f]{4,6}): UNVERIFIED", head)
    if m:
        unver.add(int(m.group(1), 16))
tested = set()
for t in ("tests/fn_auto.inc", "tests/test_fn.c", "tests/test_objects.c"):
    for name, pfx in re.findall(r'"([^"]*0x[0-9A-Fa-f]{4,6}[^"]*)",\s*"(work/ram/[^"]+)"', open(f"{ROOT}/{t}").read()):
        if has_cap(pfx):
            tested |= {int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", name)}
ver |= tested - unver
# Routines whose C ran inside those passing suites (static helpers of tested
# routines, shared code): work/gcov_exec.tsv from scripts/gcov_build.sh.
gx = f"{ROOT}/work/gcov_exec.tsv"
if os.path.exists(gx):
    ran = {int(ln.split()[0], 16) for ln in open(gx) if ln.strip() and not ln.startswith("#")}
    ver |= (ran & dec) - unver
# One-instruction routines (work/single_insn.tsv): checked against the ROM
# bytes, since no capture can tell their C from an empty body.
si = f"{ROOT}/work/single_insn.tsv"
if os.path.exists(si):
    rom = open(f"{ROOT}/work/maincpu.bin", "rb").read()
    for ln in open(si):
        if not ln.strip() or ln.startswith("#"): continue
        a, hexb = ln.split("\t")[:2]; a = int(a, 16); b = bytes.fromhex(hexb)
        if a in funcs and a in dec and rom[a:a + len(b)] == b and funcs[a][0] == len(b):
            ver.add(a)
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

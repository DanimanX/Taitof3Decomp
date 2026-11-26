#!/usr/bin/env python3
"""Build the zone-capture work list: every routine the Z01..Z27 hit survey
(work/zcap/hitsz) saw running that src/ does not yet decompile and that has
no usable capture (<10 pairs). Output TSV: addr size zone first_frame caller_ret
Usage: scripts/zcap_todo.py [--all]  (--all keeps already-captured ones)"""
import glob, re, os, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
funcs = {}
for ln in open("work/functions.tsv"):
    p = ln.split("\t")
    if not p[0].startswith("#"): funcs[int(p[0], 16)] = int(p[1])
dec = set()
for f in glob.glob("src/**/*.[ch]", recursive=True):
    dec |= {int(x, 16) for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", open(f).read())}
for f in glob.glob("src/enemy/obj_*.c"):
    if "(stub)" in open(f).read(): dec.discard(int(os.path.basename(f)[4:-2], 16))
first = {}
for f in sorted(glob.glob("work/zcap/hitsz/Z*.tsv")):
    z = os.path.basename(f)[1:3]
    for l in open(f):
        a, fr, _a0, ret = l.split()[:4]
        A, fr = int(a, 16), int(fr)
        if A not in first or fr < first[A][1]: first[A] = (z, fr, ret)
def pairs(a):
    p = f"work/ram/fn_{a:06X}_regs.tsv"
    return sum(1 for l in open(p) if "\tpost\t" in l) if os.path.exists(p) else 0
rows = [a for a in first if a not in dec and (len(sys.argv) > 1 or pairs(a) < 10)]
for a in sorted(rows, key=lambda a: -funcs.get(a, 0)):
    z, fr, ret = first[a]
    print(f"{a:06X}\t{funcs.get(a, 0)}\t{z}\t{fr}\t{ret}")

#!/usr/bin/env python3
"""zcap_accum.py ADDR [MAX] : add calls of ADDR from every zone where the hit
survey (work/zcap/hitsz/Z*.tsv) saw it, appending each new call to the existing
capture work/ram/fn_ADDR_{pairs,gfx,spr}.bin + _regs.tsv, until MAX pairs
(default 10).

Many routines are one-shot (a state's entry, called once per zone), so a single
zone run gives 1 pair however long it runs; their extra calls only exist in
other zones. Capture files are fixed-size blocks per pair, so captures are
concatenated and the regs pair index renumbered. A call whose pre-state block
and pre registers equal one already present is skipped. A capture whose file
set (gfx/spr present or not) differs from the base is not appended.
Prints: ADDR before -> after (zones tried)."""
import glob, os, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
A = sys.argv[1].upper().zfill(6)
MAX = int(sys.argv[2]) if len(sys.argv) > 2 else 10
KINDS = ("pairs", "gfx", "spr")


def load(pfx):
    p = f"{pfx}_regs.tsv"
    if not os.path.exists(p):
        return None
    lines = open(p).read().splitlines()
    n = sum(1 for l in lines[1:] if l.split("\t")[1] == "post")
    if n == 0:
        return None
    bins = {}
    for k in KINDS:
        f = f"{pfx}_{k}.bin"
        if os.path.exists(f) and os.path.getsize(f) > 0:
            d = open(f, "rb").read()
            if len(d) % n:
                return None
            bins[k] = (d, len(d) // n)
    return {"hdr": lines[0], "rows": lines[1:], "n": n, "bins": bins}


def pair_rows(c, i):
    return [r for r in c["rows"] if r.split("\t")[0] == str(i)]


def key(c, i):
    d, sz = c["bins"]["pairs"]
    pre = d[i * sz: i * sz + sz // 2]
    regs = [r.split("\t", 3)[3] for r in pair_rows(c, i) if r.split("\t")[1] == "pre"]
    return (hash(pre), tuple(regs))


def save(pfx, c):
    out = [c["hdr"]] + c["rows"]
    open(f"{pfx}_regs.tsv", "w").write("\n".join(out) + "\n")
    for k, (d, _) in c["bins"].items():
        open(f"{pfx}_{k}.bin", "wb").write(d)


base_pfx = f"work/ram/fn_{A}"
base = load(base_pfx)
before = base["n"] if base else 0
zones = []
for f in sorted(glob.glob("work/zcap/hitsz/Z*.tsv")):
    for l in open(f):
        p = l.split()
        if p and p[0] == A:
            zones.append((int(p[1]), os.path.basename(f)[1:-4]))
            break
tried = []
for fr, z in sorted(zones):
    if base and base["n"] >= MAX:
        break
    tmp = f"work/ram/zacc_{A}"
    env = dict(os.environ, ZCAP_OUT=tmp)
    subprocess.run(["scripts/zcap_one.sh", A, z, str(fr)], env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    new = load(tmp)
    tried.append(f"Z{z}:{new['n'] if new else 0}")
    for f in glob.glob(f"{tmp}_*"):
        os.remove(f)
    if not new:
        continue
    if base is None:
        base = new
        continue
    if set(new["bins"]) != set(base["bins"]):
        continue
    have = {key(base, i) for i in range(base["n"])}
    for i in range(new["n"]):
        if base["n"] >= MAX:
            break
        if key(new, i) in have:
            continue
        if any(new["bins"][k][1] != base["bins"][k][1] for k in base["bins"]):
            break
        j = base["n"]
        for r in pair_rows(new, i):
            base["rows"].append(str(j) + r[r.index("\t"):])
        for k in base["bins"]:
            d, sz = base["bins"][k]
            nd, nsz = new["bins"][k]
            base["bins"][k] = (d + nd[i * nsz:(i + 1) * nsz], sz)
        base["n"] += 1
        have.add(key(new, i))
if base and base["n"] > before:
    save(base_pfx, base)
print(f"{A} {before} -> {base['n'] if base else 0} ({' '.join(tried)})", flush=True)

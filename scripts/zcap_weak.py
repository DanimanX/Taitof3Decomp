#!/usr/bin/env python3
"""zcap_weak.py ADDR [SAMPLES] : find calls of ADDR that change something.

A "weak" capture is one where no captured call changes RAM (outside the stack
window [SP-0x100, SP)), sprite or gfx RAM, so an empty function passes it.
For every zone where the hit survey saw ADDR, capture SAMPLES calls (default
200), keep the calls that change memory (up to 12) plus up to 4 that do not,
and, if at least one changing call was found, replace work/ram/fn_ADDR_*
(the old capture is moved to work/zcap/recap/weak_old/).
Prints: ADDR effective=N kept=M (zone:effective/calls ...)."""
import glob, os, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
A = sys.argv[1].upper().zfill(6)
SAMPLES = int(sys.argv[2]) if len(sys.argv) > 2 else 200
KEEP_EFF, KEEP_NOP = 12, 4
RAM0 = 0x400000


def load(pfx):
    p = f"{pfx}_regs.tsv"
    if not os.path.exists(p):
        return None
    lines = open(p).read().splitlines()
    n = sum(1 for l in lines[1:] if l.split("\t")[1] == "post")
    if not n:
        return None
    bins = {}
    for k in ("pairs", "gfx", "spr"):
        f = f"{pfx}_{k}.bin"
        if os.path.exists(f) and os.path.getsize(f) > 0:
            d = open(f, "rb").read()
            bins[k] = (d, len(d) // n)
    return {"hdr": lines[0], "rows": lines[1:], "n": n, "bins": bins}


def rows(c, i):
    return [r for r in c["rows"] if r.split("\t")[0] == str(i)]


def effective(c, i):
    rr = {r.split("\t")[1]: r.split("\t") for r in rows(c, i)}
    hdr = c["hdr"].split("\t")
    # the test compares D0, D1, A0, A1 at return (ALLREGS)
    for reg in ("D0", "D1", "A0", "A1"):
        k = hdr.index(reg)
        if rr["pre"][k] != rr["post"][k]:
            return True
    sp = int(rr["pre"][-1], 16)
    for k, (d, sz) in c["bins"].items():
        h = sz // 2
        pre, post = d[i * sz: i * sz + h], d[i * sz + h: (i + 1) * sz]
        if pre == post:
            continue
        if k != "pairs":
            return True
        lo, hi = sp - 0x100 - RAM0, sp - RAM0
        for j in range(len(pre)):
            if pre[j] != post[j] and not (lo <= j < hi):
                return True
    return False


zones = []
for f in sorted(glob.glob("work/zcap/hitsz/Z*.tsv")):
    for l in open(f):
        p = l.split()
        if p and p[0] == A:
            zones.append((int(p[1]), os.path.basename(f)[1:-4]))
            break
eff, nop, info, kinds = [], [], [], None
for fr, z in sorted(zones):
    if len(eff) >= KEEP_EFF:
        break
    tmp = f"work/ram/zweak_{A}_{z}"
    env = dict(os.environ, ZCAP_OUT=tmp, ZCAP_SAMPLES=str(SAMPLES))
    subprocess.run(["scripts/zcap_one.sh", A, z, str(fr)], env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    c = load(tmp)
    if not c:
        info.append(f"Z{z}:0")
        continue
    if kinds is None:
        kinds = set(c["bins"])
    ne = 0
    if set(c["bins"]) == kinds:
        for i in range(c["n"]):
            if effective(c, i):
                ne += 1
                if len(eff) < KEEP_EFF:
                    eff.append((c, i))
            elif len(nop) < KEEP_NOP:
                nop.append((c, i))
    info.append(f"Z{z}:{ne}/{c['n']}")
    c["tmp"] = tmp
    if ne == 0:
        for f in glob.glob(f"{tmp}_*"):
            os.remove(f)

if eff:
    out_rows, out_bins, j = [], {k: b"" for k in kinds}, 0
    for c, i in eff + nop:
        for r in rows(c, i):
            out_rows.append(str(j) + r[r.index("\t"):])
        for k in kinds:
            d, sz = c["bins"][k]
            out_bins[k] += d[i * sz:(i + 1) * sz]
        j += 1
    os.makedirs("work/zcap/recap/weak_old", exist_ok=True)
    for f in glob.glob(f"work/ram/fn_{A}_*"):
        shutil.move(f, f"work/zcap/recap/weak_old/{os.path.basename(f)}")
    hdr = eff[0][0]["hdr"]
    open(f"work/ram/fn_{A}_regs.tsv", "w").write("\n".join([hdr] + out_rows) + "\n")
    for k, d in out_bins.items():
        open(f"work/ram/fn_{A}_{k}.bin", "wb").write(d)
for f in glob.glob(f"work/ram/zweak_{A}_*"):
    os.remove(f)
print(f"{A} effective={len(eff)} kept={len(eff) + len(nop) if eff else 0} ({' '.join(info)})", flush=True)

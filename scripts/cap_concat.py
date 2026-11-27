#!/usr/bin/env python3
"""cap_concat.py OUT IN1 [IN2 ...] : concatenate capture_fn captures (prefixes)
into OUT (fixed-size blocks per pair, pair index renumbered, duplicate pre
states skipped; every input must have the same file set and header)."""
import os, sys
out, ins = sys.argv[1], sys.argv[2:]
hdr, rows, bins, seen, j = None, [], {}, set(), 0
for p in ins:
    if not os.path.exists(p + "_regs.tsv"):
        continue
    L = open(p + "_regs.tsv").read().splitlines()
    n = sum(1 for l in L[1:] if l.split("\t")[1] == "post")
    if not n:
        continue
    if hdr is None:
        hdr = L[0]
    assert L[0] == hdr, p
    data = {k: open(f"{p}_{k}.bin", "rb").read() for k in ("pairs", "gfx", "spr")
            if os.path.exists(f"{p}_{k}.bin") and os.path.getsize(f"{p}_{k}.bin")}
    for i in range(n):
        sz = len(data["pairs"]) // n
        pre = data["pairs"][i * sz: i * sz + sz // 2]
        if pre in seen:
            continue
        seen.add(pre)
        for r in L[1:]:
            if r.split("\t")[0] == str(i) and r.split("\t")[1] in ("pre", "post"):
                rows.append(str(j) + r[r.index("\t"):])
        for k, d in data.items():
            s = len(d) // n
            bins[k] = bins.get(k, b"") + d[i * s:(i + 1) * s]
        j += 1
open(out + "_regs.tsv", "w").write("\n".join([hdr] + rows) + "\n")
for k, d in bins.items():
    open(f"{out}_{k}.bin", "wb").write(d)
print(out, j, "pairs")

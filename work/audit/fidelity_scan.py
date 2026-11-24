#!/usr/bin/env python3
"""Classify capture fidelity for object decompilation captures.

For each routine address given on the command line (or every obj_*.c in
src/enemy/ that still contains "not decompiled yet (stub)"), reads the
paired binary and register-TSV dumps and reports:

  - total pair count
  - whether obj+0 stores the routine address in every snapshot
  - histogram of the state byte at obj+6
  - count of distinct aligned-2-byte offsets within the first 0x6C bytes
    of the object that change in at least one pair (pre vs post)

Output: one TSV line per routine, exit code 0.
"""
import os
import sys
import glob
from collections import Counter

W = 128 * 1024  # WRAM bytes per snapshot

# Paths are relative to repo root (grandparent of this file)
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def find_stubs():
    """Return sorted list of hex addresses from obj_*.c containing the stub marker."""
    out = []
    for src in sorted(glob.glob(os.path.join(ROOT, "src/enemy/obj_*.c"))):
        if "not decompiled yet (stub)" in open(src).read():
            addr = os.path.basename(src)[4:-2]  # strip 'obj_' and '.c'
            out.append(addr)
    return out


def read_regs(path):
    """Return dict mapping (pair_index, 'pre'/'post') -> list of tab columns."""
    meta = {}
    for line in open(path).read().splitlines()[1:]:
        parts = line.split("\t")
        if len(parts) >= 12:
            pair_idx = int(parts[0])
            which = parts[1]
            meta[(pair_idx, which)] = parts
    return meta


def read_pairs(path):
    """Return list of (pair_index, pre_bytes, post_bytes)."""
    data = open(path, "rb").read()
    n = len(data) // (2 * W)
    out = []
    for i in range(n):
        pre = data[i * 2 * W : i * 2 * W + W]
        post = data[(i * 2 + 1) * W : (i * 2 + 2) * W]
        out.append((i, pre, post))
    return out


def analyze(addr):
    """Run fidelity checks for one routine; returns a dict or None."""
    pairs_path = os.path.join(ROOT, "work/ram/obj_%s_pairs.bin" % addr)
    regs_path = os.path.join(ROOT, "work/ram/obj_%s_regs.tsv" % addr)

    if not (os.path.exists(pairs_path) and os.path.exists(regs_path)):
        return None

    meta = read_regs(regs_path)
    pairs = read_pairs(pairs_path)
    routine_addr = int(addr, 16)

    valid_count = 0
    state_hist = Counter()
    fn_ok_all = True
    changed_offsets = set()  # distinct even offsets within [0, 0x6C] that change

    for pair_idx, pre, post in pairs:
        key_pre = (pair_idx, "pre")
        if key_pre not in meta:
            continue

        parts = meta[key_pre]
        a0_str = parts[11]  # column index 11 is A0
        try:
            obj_addr = int(a0_str, 16) - 0x400000
        except ValueError:
            continue

        if not (0 <= obj_addr < W):
            continue

        valid_count += 1

        # State byte at obj+6
        state_byte = pre[obj_addr + 6]
        state_hist[state_byte] += 1

        # Check fn(obj)==routine address in every snapshot pair
        for snap in (pre, post):
            val = int.from_bytes(
                snap[obj_addr : obj_addr + 4], "big"
            )
            if val != routine_addr:
                fn_ok_all = False
                break

        # Check aligned 2-byte fields inside the first 0x6C bytes of object
        for off in range(0, 0x6C + 1, 2):
            pre_val = int.from_bytes(pre[obj_addr + off : obj_addr + off + 2], "big")
            post_val = int.from_bytes(post[obj_addr + off : obj_addr + off + 2], "big")
            if pre_val != post_val:
                changed_offsets.add(off)

    return {
        "addr": addr,
        "pairs": valid_count,
        "fn_ok": "yes" if fn_ok_all else "no",
        "state_hist": dict(state_hist),
        "field_changes": len(changed_offsets),
    }


def main():
    if len(sys.argv) > 1:
        addrs = sys.argv[1:]
    else:
        addrs = find_stubs()

    # Print header
    print("addr\tpairs\tfn_ok\tstate_hist\tfield_changes")

    for addr in sorted(addrs):
        result = analyze(addr)
        if not result or result["pairs"] == 0:
            continue

        hist_str = ";".join(
            "%s:%d" % (format(k, "02x"), v) for k, v in sorted(result["state_hist"].items())
        )

        print("%s\t%d\t%s\t%s\t%d" % (result["addr"], result["pairs"], result["fn_ok"], hist_str, result["field_changes"]))

    sys.exit(0)


if __name__ == "__main__":
    main()

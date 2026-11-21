#!/usr/bin/env python3
"""Correlate per-frame Work RAM word deltas with the P1 input schedule
(scripts/lua/session_corr.lua output) to locate player X/Y and fire state.
Usage: scripts/ramcorr.py [work/ram]
"""
import sys
import numpy as np

d = sys.argv[1] if len(sys.argv) > 1 else "work/ram"
raw = np.fromfile(d + "/corr_frames.bin", dtype=">i2").reshape(-1, 0x10000).astype(np.int32)
inp = [l.split("\t")[1].strip() for l in open(d + "/corr_inputs.tsv").readlines()[1:]]
T = raw.shape[0]
delta = np.diff(raw, axis=0)
delta = ((delta + 0x8000) % 0x10000) - 0x8000  # wrap-aware int16 delta
sig = {
    "X(R+,L-)": np.array([1 if k == "R" else -1 if k == "L" else 0 for k in inp], float),
    "Y(D+,U-)": np.array([1 if k == "D" else -1 if k == "U" else 0 for k in inp], float),
    "FIRE": np.array([1 if k == "F" else 0 for k in inp], float),
}
def corr(sig_vec, use_delta=True):
    best = np.zeros(0x10000); lag_best = np.zeros(0x10000, int)
    for lag in range(0, 4):
        s = sig_vec[1:T] if use_delta else sig_vec
        s = np.roll(s, lag); s[:lag] = 0
        m = delta if use_delta else raw
        ms = m - m.mean(0); ss = s - s.mean()
        num = (ms * ss[:, None]).sum(0)
        den = np.sqrt((ms ** 2).sum(0) * (ss ** 2).sum()) + 1e-9
        c = num / den
        upd = np.abs(c) > np.abs(best)
        best[upd] = c[upd]; lag_best[upd] = lag
    return best, lag_best
for name, s in sig.items():
    c, lag = corr(s, use_delta=(name != "FIRE"))
    top = np.argsort(-np.abs(c))[:8]
    print("== %s ==" % name)
    for i in top:
        col = raw[:, i]
        print("  %06X corr=%+.3f lag=%d range=[%d..%d]" % (0x400000 + 2 * i, c[i], lag[i], col.min(), col.max()))

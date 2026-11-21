#!/usr/bin/env python3
"""Aggregate a mame-mcp trace log per (R/W, PC, region): address span, distinct
addresses, total hits. Output TSV is small enough to commit; raw logs of the
video sweep are multi-MB.

Usage: scripts/summarize_trace.py traces/mmio/sweepB_video.log > traces/mmio/sweepB_video.pcs.tsv
"""
import sys
from collections import defaultdict

agg = defaultdict(lambda: [None, None, 0, 0])
for line in open(sys.argv[1]):
    p = line.split()
    if len(p) < 5 or p[0] not in ("R", "W"):
        continue
    k = (p[0], int(p[1], 16), p[3])
    a, hits = int(p[2], 16), int(p[4])
    g = agg[k]
    g[0] = a if g[0] is None else min(g[0], a)
    g[1] = a if g[1] is None else max(g[1], a)
    g[2] += 1
    g[3] += hits
print("rw\tpc\tregion\tlo\thi\tn_addrs\thits")
for (rw, pc, rng), (lo, hi, n, hits) in sorted(agg.items(), key=lambda kv: (kv[0][1], kv[0][0])):
    print("%s\t%06X\t%s\t%06X\t%06X\t%d\t%d" % (rw, pc, rng, lo, hi, n, hits))

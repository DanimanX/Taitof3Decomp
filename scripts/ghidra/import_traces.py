# Import MMIO trace summaries (traces/mmio/*.pcs.tsv, made by
# scripts/summarize_trace.py from mame-mcp logs) as EOL comments + bookmarks.
# TSV columns: rw pc region lo hi n_addrs hits
# @category Taitof3Decomp
# @runtime PyGhidra
import glob
import os
from collections import defaultdict
from ghidra.program.model.listing import CodeUnit

prog = currentProgram  # noqa: F821
space = prog.getAddressFactory().getDefaultAddressSpace()
listing = prog.getListing()
bm = prog.getBookmarkManager()
fm = prog.getFunctionManager()
root = os.environ.get("TAITOF3_ROOT", "/PROCESOS/Taitof3Decomp")

by_pc = defaultdict(list)
for path in sorted(glob.glob(os.path.join(root, "traces/mmio/*.pcs.tsv"))):
    tag = os.path.basename(path).split(".")[0]
    for line in open(path):
        p = line.rstrip("\n").split("\t")
        if len(p) < 7 or p[0] not in ("R", "W"):
            continue
        by_pc[int(p[1], 16)].append((p[0], int(p[3], 16), int(p[4], 16), p[2], int(p[6]), tag))

MARK = "[mmio] "
for pc, accs in sorted(by_pc.items()):
    a = space.getAddress(pc)
    # aggregate per (rw, region): address span + total hits
    agg = defaultdict(lambda: [None, None, 0])
    for rw, lo, hi, rng, hits, tag in accs:
        g = agg[(rw, rng)]
        g[0] = lo if g[0] is None else min(g[0], lo)
        g[1] = hi if g[1] is None else max(g[1], hi)
        g[2] += hits
    parts = []
    for (rw, rng), (lo, hi, hits) in sorted(agg.items()):
        span = "%06X" % lo if lo == hi else "%06X-%06X" % (lo, hi)
        parts.append("%s %s %s x%d" % (rw, rng, span, hits))
    text = MARK + "; ".join(parts)
    old = listing.getComment(CodeUnit.EOL_COMMENT, a) or ""
    kept = [l for l in old.splitlines() if not l.startswith(MARK)]
    listing.setComment(a, CodeUnit.EOL_COMMENT, "\n".join(kept + [text]))
    bm.setBookmark(a, "Analysis", "MMIO", text)
    if listing.getInstructionAt(a) is None:
        disassemble(a)  # noqa: F821
    if fm.getFunctionContaining(a) is None:
        print("PC %06X not inside a function" % pc)
print("import_traces: %d PCs annotated" % len(by_pc))

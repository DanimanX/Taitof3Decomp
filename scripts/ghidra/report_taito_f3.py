# Phase-2 gate report: function count + XREF counts into each HW block.
# @category Taitof3Decomp
# @runtime PyGhidra
prog = currentProgram  # noqa: F821
mem = prog.getMemory()
rm = prog.getReferenceManager()
fm = prog.getFunctionManager()
print("functions: %d" % fm.getFunctionCount())
for b in mem.getBlocks():
    if b.getName() == "ROM":
        continue
    refs, froms = 0, set()
    it = rm.getReferenceDestinationIterator(prog.getAddressFactory().getAddressSet(b.getStart(), b.getEnd()), True)
    for a in it:
        for r in rm.getReferencesTo(a):
            refs += 1
            f = fm.getFunctionContaining(r.getFromAddress())
            froms.add(f.getName() if f else str(r.getFromAddress()))
    print("%-16s %s-%s refs=%5d from_funcs=%d" % (b.getName(), b.getStart(), b.getEnd(), refs, len(froms)))

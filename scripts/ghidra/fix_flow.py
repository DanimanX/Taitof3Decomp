# Repair code flow broken by spurious data (e.g. pointers created over code at
# 0x1019E in Task0_Main). For every instruction whose fall-through lands on
# data/undefined bytes inside ROM, clear the data and disassemble there.
# Iterates to a fixed point, then fixes up the bodies of affected functions.
# NOTE: do not call analyzeChanges() afterwards: an auto-analyzer re-clears
# these fall-throughs (observed at 0x1019C), undoing the repair.
# @category Taitof3Decomp
# @runtime PyGhidra
from ghidra.app.cmd.disassemble import DisassembleCommand
from ghidra.program.model.address import AddressSet

prog = currentProgram  # noqa: F821
L = prog.getListing()
mem = prog.getMemory()
rom = mem.getBlock("ROM")
total, changed, funcs = 0, AddressSet(), set()
def conflict_targets():
    """Addresses from Ghidra 'Bad Instruction' bookmarks: 'Failed to
    disassemble at X due to conflicting data at Y (flow from Z)'."""
    out = []
    bm = prog.getBookmarkManager()
    it = bm.getBookmarksIterator("Error")
    while it.hasNext():
        b = it.next()
        if "conflicting data" in (b.getComment() or "") and rom.contains(b.getAddress()):
            out.append(b)
    return out


for it in range(20):
    fixes = []
    for b in conflict_targets():
        if L.getInstructionAt(b.getAddress()) is None:
            fixes.append(b.getAddress())
        prog.getBookmarkManager().removeBookmark(b)
    for ins in L.getInstructions(rom.getStart(), True):
        ft = ins.getFallThrough()
        if ft is None or not rom.contains(ft) or L.getInstructionAt(ft) is not None:
            continue
        cu = L.getCodeUnitContaining(ft)
        if cu is None or L.getInstructionContaining(ft) is not None:
            continue
        fixes.append(ft)
    if not fixes:
        break
    n = 0
    for ft in fixes:
        if L.getInstructionAt(ft) is not None:
            continue
        cu = L.getCodeUnitContaining(ft)
        start = cu.getMinAddress() if cu is not None else ft
        if start.compareTo(ft) < 0:
            start = ft
        end = ft.add(15)
        L.clearCodeUnits(start, end, False)
        cmd = DisassembleCommand(ft, None, True)
        if cmd.applyTo(prog) and L.getInstructionAt(ft) is not None:
            n += 1
            changed.add(ft)
            src = L.getInstructionBefore(ft)
            f = getFunctionContaining(src.getAddress()) if src is not None else None  # noqa: F821
            if f is not None:
                funcs.add(f)
    total += n
    print("fix_flow pass %d: %d fall-throughs repaired" % (it, n))
    if n == 0:
        break
print("fix_flow: %d repairs" % total)
from ghidra.app.cmd.function import CreateFunctionCmd
for f in funcs:
    CreateFunctionCmd.fixupFunctionBody(prog, f, monitor)  # noqa: F821
print("fix_flow: %d function bodies fixed up" % len(funcs))

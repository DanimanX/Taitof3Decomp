# Seed a program from MAME traces (scripts/trace_pcs.py output): disassemble
# at every executed PC (<prefix>.pcs) and create a function at every call
# target (<prefix>.calls). Several prefixes may be given; files that do not
# exist are skipped. Run before fix_flow.py and export_functions.py.
# Args: <prefix> [<prefix> ...]
# @category Taitof3Decomp
# @runtime PyGhidra
import os
from ghidra.app.cmd.disassemble import DisassembleCommand
from ghidra.app.cmd.function import CreateFunctionCmd
from ghidra.program.model.address import AddressSet

prog = currentProgram  # noqa: F821
sp = prog.getAddressFactory().getDefaultAddressSpace()
L = prog.getListing()
FM = prog.getFunctionManager()
pcs, calls = set(), set()
for pfx in getScriptArgs():  # noqa: F821
    for ext, dst in (("pcs", pcs), ("calls", calls)):
        p = pfx + "." + ext
        if os.path.exists(p):
            for ln in open(p):
                if ln.strip():
                    dst.add(int(ln, 16))
todo = AddressSet()
for a in sorted(pcs | calls):
    if L.getInstructionAt(sp.getAddress(a)) is None:
        todo.add(sp.getAddress(a))
cmd = DisassembleCommand(todo, None, True)
cmd.applyTo(prog, monitor)  # noqa: F821
# direct jsr/bsr targets inside the newly disassembled code are entries too
# (no auto-analysis runs in this pass to create them)
it = L.getInstructions(sp.getAddress(0), True)
while it.hasNext():
    ins = it.next()
    if ins.getAddress().getOffset() > 0x1FFFFF:
        break
    if ins.getMnemonicString().lower().startswith(("jsr", "bsr")):
        for r in ins.getReferencesFrom():
            t = r.getToAddress().getOffset()
            if r.getReferenceType().isCall() and t < 0x200000:
                calls.add(t)
todo2 = AddressSet()
for a in calls:
    if L.getInstructionAt(sp.getAddress(a)) is None:
        todo2.add(sp.getAddress(a))
DisassembleCommand(todo2, None, True).applyTo(prog, monitor)  # noqa: F821
made = 0
for a in sorted(calls):
    ad = sp.getAddress(a)
    if FM.getFunctionAt(ad) is None and L.getInstructionAt(ad) is not None:
        if CreateFunctionCmd(None, ad, None, None).applyTo(prog, monitor):  # noqa: F821
            made += 1
print("seed_trace: %d executed PCs, %d call targets, %d new functions" % (len(pcs), len(calls), made))
# Orphan code: disassembled instructions in no function (reached by jmp,
# rte/rts dispatch or a task switch). Make a function at the start of every
# run of them, repeating until each is covered.
orph = 0
for _ in range(10):
    starts, prev_in = [], True
    it = L.getInstructions(sp.getAddress(0), True)
    last_end = -1
    while it.hasNext():
        ins = it.next()
        a = ins.getAddress().getOffset()
        if a > 0x1FFFFF:
            break
        inside = FM.getFunctionContaining(ins.getAddress()) is not None
        if not inside and (prev_in or a != last_end):
            starts.append(ins.getAddress())
        prev_in = inside
        last_end = a + ins.getLength()
    if not starts:
        break
    n0 = orph
    for ad in starts:
        if FM.getFunctionContaining(ad) is None and CreateFunctionCmd(None, ad, None, None).applyTo(prog, monitor):  # noqa: F821
            orph += 1
    if orph == n0:
        break
print("seed_trace: %d functions for orphan code" % orph)

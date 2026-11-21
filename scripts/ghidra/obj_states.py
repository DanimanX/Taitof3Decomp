# Discover object tick routines ("states") that are only referenced as
# immediates: the object pool calls (obj+0) indirectly, and routines switch
# state with `move.l #next,(A0)` (0x20BC imm32) or pass the initial routine to
# Obj_Spawn via `move.l #fn,-(SP)` (0x2F3C imm32) shortly before
# `jsr Obj_Spawn` (0x4EB9 0001057C). Disassembles each target, creates a
# function OBJ_<addr> unless named, and iterates to a fixed point (newly
# disassembled code reveals more states). Also seeds the tick routines
# observed in MAME (traces/obj_ticks.txt: one hex address per line; some are
# only referenced from data tables). Run after fix_flow, -noanalysis.
# @category Taitof3Decomp
# @runtime PyGhidra
from ghidra.app.cmd.disassemble import DisassembleCommand
from ghidra.program.model.symbol import SourceType

prog = currentProgram  # noqa: F821
L = prog.getListing()
mem = prog.getMemory()
rom = mem.getBlock("ROM")
fm = prog.getFunctionManager()
space = prog.getAddressFactory().getDefaultAddressSpace()
ROM_END = 0x1FFFFF
OBJ_SPAWN = 0x1057C


def u16(a):
    return mem.getShort(space.getAddress(a)) & 0xFFFF


def u32(a):
    return mem.getInt(space.getAddress(a)) & 0xFFFFFFFF


def candidates():
    out = set()
    for ins in L.getInstructions(rom.getStart(), True):
        a = ins.getAddress().getOffset()
        op = u16(a)
        if op == 0x20BC:                                # move.l #imm,(A0)
            out.add(u32(a + 2))
        elif op == 0x2F3C:                              # move.l #imm,-(SP)
            nxt = ins
            for _ in range(6):
                nxt = nxt.getNext()
                if nxt is None:
                    break
                na = nxt.getAddress().getOffset()
                if u16(na) == 0x4EB9 and u32(na + 2) == OBJ_SPAWN:
                    out.add(u32(a + 2))
                    break
    import os
    tr = os.path.join(os.path.dirname(getSourceFile().getAbsolutePath()), "..", "..", "traces", "obj_ticks.txt")  # noqa: F821
    if os.path.exists(tr):
        for line in open(tr):
            if line.strip():
                out.add(int(line.split()[0], 16))
    return set(t for t in out if 0x400 <= t <= ROM_END and not t & 1)


done, created = set(), 0
for it in range(30):
    new = [t for t in sorted(candidates()) if t not in done]
    if not new:
        break
    n = 0
    for t in new:
        done.add(t)
        ta = space.getAddress(t)
        if L.getInstructionAt(ta) is None:
            if L.getInstructionContaining(ta) is not None:
                print("obj_states: %06X is inside an instruction, skipped" % t)
                continue
            L.clearCodeUnits(ta, ta.add(1), False)
            if not DisassembleCommand(ta, None, True).applyTo(prog) or L.getInstructionAt(ta) is None:
                print("obj_states: %06X failed to disassemble" % t)
                continue
        if fm.getFunctionAt(ta) is None:
            f = createFunction(ta, "OBJ_%06X" % t)  # noqa: F821
            if f is None:
                prog.getSymbolTable().createLabel(ta, "OBJ_%06X" % t, SourceType.ANALYSIS)
            n += 1
    created += n
    print("obj_states pass %d: %d targets, %d new" % (it, len(new), n))
print("obj_states: %d object state routines discovered" % created)

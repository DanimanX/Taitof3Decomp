# Register every src/enemy/obj_XXXXXX.c entry as a Ghidra function (OBJ_<addr>)
# and export work/functions.tsv again, so the coverage map counts routines that
# were discovered by MAME captures / subagent batches rather than by
# obj_states.py at bootstrap time. Run against a COPY of the project
# (GHIDRA_PROJDIR); the real project stays untouched until the copy is diffed.
# @category Taitof3Decomp
# @runtime PyGhidra
import glob
import os

prog = currentProgram  # noqa: F821
L = prog.getListing()
fm = prog.getFunctionManager()
space = prog.getAddressFactory().getDefaultAddressSpace()
st = prog.getSymbolTable()

ROOT = "/PROCESOS/Taitof3Decomp"
made = 0
for p in sorted(glob.glob(ROOT + "/src/enemy/obj_*.c")):
    name = os.path.basename(p)[4:-2]
    try:
        addr = int(name, 16)
    except ValueError:
        continue          # obj_E02C4 etc are fine; anything else is not ours
    a = space.getAddress(addr)
    if fm.getFunctionAt(a) is not None:
        continue
    cmd = createFunction(a, "OBJ_%05X" % addr)  # noqa: F821
    if cmd:
        made += 1
    else:
        print("FAILED to create OBJ_%05X" % addr)
print("register_obj_entries: created %d functions" % made)

# same format as export_functions.py, so scripts/coverage.py reads it as-is
out = ROOT + "/work/functions.tsv.new"
ROM_END = 0x1FFFFF
fh = open(out, "w")
for f in fm.getFunctions(True):
    e = f.getEntryPoint().getOffset()
    if e > ROM_END:
        continue
    n = 0
    for ins in L.getInstructions(f.getBody(), True):
        n += ins.getLength()
    fh.write("%06X\t%d\t%s\n" % (e, n, f.getName()))
tot = 0
it = L.getInstructions(space.getAddress(0), True)
while it.hasNext():
    ins = it.next()
    if ins.getAddress().getOffset() > ROM_END:
        break
    tot += ins.getLength()
fh.write("#total_insn_bytes\t%d\n" % tot)
fh.close()
print("export_functions: %d bytes total -> %s" % (tot, out))

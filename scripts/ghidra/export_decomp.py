# Export Ghidra pseudo-C for every function to work/decomp/<addr>_<name>.c
# plus work/decomp/INDEX.tsv (addr, name, size, callers, callees, mmio blocks touched).
# Generated output is derived from the ROM: keep it out of git.
# @category Taitof3Decomp
# @runtime PyGhidra
import os
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

prog = currentProgram  # noqa: F821
root = os.environ.get("TAITOF3_ROOT", "/PROCESOS/Taitof3Decomp")
out = os.path.join(root, "work/decomp")
os.makedirs(out, exist_ok=True)
mem = prog.getMemory()
rm = prog.getReferenceManager()
mon = ConsoleTaskMonitor()
di = DecompInterface()
di.openProgram(prog)

idx = open(os.path.join(out, "INDEX.tsv"), "w")
idx.write("addr\tname\tbytes\tcallers\tcallees\tregions\n")
n = 0
for f in prog.getFunctionManager().getFunctions(True):
    ep = f.getEntryPoint()
    regions = set()
    for rng in f.getBody():
        a = rng.getMinAddress()
        it = rm.getReferenceSourceIterator(f.getBody(), True)
        break
    for src in rm.getReferenceSourceIterator(f.getBody(), True):
        for r in rm.getReferencesFrom(src):
            b = mem.getBlock(r.getToAddress())
            if b is not None and b.getName() != "ROM":
                regions.add(b.getName())
    res = di.decompileFunction(f, 60, mon)
    code = res.getDecompiledFunction().getC() if res and res.decompileCompleted() else "/* decompile failed: %s */\n" % (res.getErrorMessage() if res else "?")
    name = "%06X_%s.c" % (ep.getOffset(), f.getName())
    with open(os.path.join(out, name), "w") as fh:
        fh.write("/* ROM: 0x%06X  %s */\n" % (ep.getOffset(), f.getName()))
        fh.write(code)
    idx.write("%06X\t%s\t%d\t%d\t%d\t%s\n" % (
        ep.getOffset(), f.getName(), f.getBody().getNumAddresses(),
        len(f.getCallingFunctions(mon)), len(f.getCalledFunctions(mon)),
        ",".join(sorted(regions))))
    n += 1
idx.close()
print("export_decomp: %d functions -> %s" % (n, out))

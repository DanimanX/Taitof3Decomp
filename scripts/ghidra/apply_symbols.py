# Apply symbols/<game>.tsv (addr kind name size comment) to the program.
# Script arg: path to tsv (default symbols/dariusg.tsv). Idempotent.
# @category Taitof3Decomp
# @runtime PyGhidra
import os
from ghidra.program.model.symbol import SourceType
from ghidra.program.model.data import ByteDataType, WordDataType, DWordDataType, ArrayDataType
from ghidra.program.model.listing import CodeUnit

prog = currentProgram  # noqa: F821
args = getScriptArgs()  # noqa: F821
root = os.environ.get("TAITOF3_ROOT", "/PROCESOS/Taitof3Decomp")
path = args[0] if args else os.path.join(root, "symbols/dariusg.tsv")
sp = prog.getAddressFactory().getDefaultAddressSpace()
st = prog.getSymbolTable()
fm = prog.getFunctionManager()
listing = prog.getListing()
mem = prog.getMemory()
VOL = []
n = 0
for line in open(path):
    if not line.strip() or line.startswith("#"):
        continue
    p = line.rstrip("\n").split("\t")
    addr, kind, name, size = int(p[0], 16), p[1], p[2], int(p[3])
    cmt = p[4] if len(p) > 4 else ""
    a = sp.getAddress(addr)
    if kind == "func":
        f = fm.getFunctionAt(a)
        if f is None:
            disassemble(a)  # noqa: F821
            f = createFunction(a, name)  # noqa: F821
        if f is not None:
            f.setName(name, SourceType.USER_DEFINED)
        else:
            print("warn: no function at %06X" % addr)
    else:
        st.createLabel(a, name, SourceType.USER_DEFINED).setPrimary()
        if kind == "data" and size in (1, 2, 4) and mem.getBlock(a) is not None:
            dt = {1: ByteDataType(), 2: WordDataType(), 4: DWordDataType()}[size]
            try:
                listing.clearCodeUnits(a, a.add(size - 1), False)
                listing.createData(a, dt)
            except Exception as e:  # noqa: BLE001
                print("warn: data %06X: %s" % (addr, e))
    if cmt:
        listing.setComment(a, CodeUnit.PLATE_COMMENT, cmt)
        if "VOLATILE" in cmt:
            VOL.append((a, size, name))
    n += 1
# VOLATILE vars: split them into their own volatile memory block so the
# decompiler keeps spin-waits on them (e.g. the IRQ2<->IRQ3 handshake).
for a, size, name in VOL:
    b = mem.getBlock(a)
    if b is None or b.isVolatile():
        continue
    if b.getStart() != a:
        mem.split(b, a)
        b = mem.getBlock(a)
    end = a.add(size)
    if b.getEnd().compareTo(end) >= 0:
        mem.split(b, end)
        b = mem.getBlock(a)
    b.setName("VOL_" + name)
    b.setVolatile(True)
    print("volatile block %s" % b.getName())
print("apply_symbols: %d entries from %s" % (n, path))

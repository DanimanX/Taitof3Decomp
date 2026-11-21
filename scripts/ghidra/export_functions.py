# Export every function in maincpu ROM with its instruction-byte size, plus
# the ROM total of disassembled instruction bytes (for coverage reports).
# Args: <out_tsv>   -> lines "entry_hex<TAB>code_bytes<TAB>name", last line
#   "#total_insn_bytes<TAB>N"
# @category Taitof3Decomp
# @runtime PyGhidra
args = getScriptArgs()  # noqa: F821
prog = currentProgram  # noqa: F821
L = prog.getListing()
ROM_END = 0x1FFFFF
out = open(args[0], "w")
for f in prog.getFunctionManager().getFunctions(True):
    e = f.getEntryPoint().getOffset()
    if e > ROM_END:
        continue
    n = 0
    for ins in L.getInstructions(f.getBody(), True):
        n += ins.getLength()
    out.write("%06X\t%d\t%s\n" % (e, n, f.getName()))
tot = 0
sp = prog.getAddressFactory().getDefaultAddressSpace()
it = L.getInstructions(sp.getAddress(0), True)
while it.hasNext():
    ins = it.next()
    if ins.getAddress().getOffset() > ROM_END:
        break
    tot += ins.getLength()
out.write("#total_insn_bytes\t%d\n" % tot)
out.close()
print("export_functions: done")

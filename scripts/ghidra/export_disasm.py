# Dump disassembly listing for address ranges to stdout-file.
# Args (script args): <out_path> <start_hex> <end_hex> [<start_hex> <end_hex> ...]
# @category Taitof3Decomp
# @runtime PyGhidra
args = getScriptArgs()  # noqa: F821
prog = currentProgram  # noqa: F821
sp = prog.getAddressFactory().getDefaultAddressSpace()
listing = prog.getListing()
fm = prog.getFunctionManager()
with open(args[0], "w") as out:
    for i in range(1, len(args), 2):
        lo, hi = int(args[i], 16), int(args[i + 1], 16)
        for cu in listing.getCodeUnits(prog.getAddressFactory().getAddressSet(sp.getAddress(lo), sp.getAddress(hi)), True):
            a = cu.getAddress()
            f = fm.getFunctionAt(a)
            if f is not None:
                out.write("\n%s:\n" % f.getName())
            else:
                s = prog.getSymbolTable().getPrimarySymbol(a)
                if s is not None and not s.getName().startswith("LAB_"):
                    out.write("%s:\n" % s.getName())
                elif s is not None:
                    out.write("%s:\n" % s.getName())
            c = cu.getComment(0)  # EOL
            out.write("  %06X  %-40s%s\n" % (a.getOffset(), cu.toString(), ("  ; " + c.replace("\n", " | ")) if c else ""))
print("export_disasm: wrote " + args[0])

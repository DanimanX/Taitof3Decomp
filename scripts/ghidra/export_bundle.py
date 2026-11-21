# Export a routine plus everything it reaches (calls, jumps, tail jumps)
# up to a depth, as one disassembly file for delegation (agy/hermes).
# Follows instruction flow from the root address (not Ghidra function
# bodies, which are unreliable for object state routines), so each "block"
# is a maximal straight run of instructions ending at rts/rte/jmp/bra.
# Args: <out_path> <root_hex> [max_depth=3] [stop_hex,stop_hex,...]
#   stop addresses (already-decompiled routines) are listed but not expanded.
# @category Taitof3Decomp
# @runtime PyGhidra
args = getScriptArgs()  # noqa: F821
prog = currentProgram  # noqa: F821
sp = prog.getAddressFactory().getDefaultAddressSpace()
L = prog.getListing()
mem = prog.getMemory()
st = prog.getSymbolTable()
root = int(args[1], 16)
max_depth = int(args[2]) if len(args) > 2 else 3
stops = set(int(x, 16) for x in args[3].split(",")) if len(args) > 3 and args[3] else set()
ROM_END = 0x1FFFFF


def name_at(a):
    s = st.getPrimarySymbol(sp.getAddress(a))
    return s.getName() if s is not None else "%06X" % a


seen_insn = set()
blocks = []          # (start, [lines])
queue = [(root, 0)]
called = {}
while queue:
    start, depth = queue.pop(0)
    if start in seen_insn or start > ROM_END:
        continue
    if start in stops and start != root:
        called[start] = "STOP (already decompiled)"
        continue
    lines, a = [], start
    while True:
        ins = L.getInstructionAt(sp.getAddress(a))
        if ins is None and L.getInstructionContaining(sp.getAddress(a)) is None:
            # jump-table targets and code after calls Ghidra thought were
            # non-returning are left undecoded: disassemble in memory
            # (the project is opened read-only, nothing is saved)
            from ghidra.app.cmd.disassemble import DisassembleCommand
            d = L.getDataContaining(sp.getAddress(a))
            lo = d.getMinAddress() if d is not None else sp.getAddress(a)
            L.clearCodeUnits(lo, sp.getAddress(a + 0x20), False)
            DisassembleCommand(sp.getAddress(a), None, True).applyTo(prog)
            ins = L.getInstructionAt(sp.getAddress(a))
        if ins is None:
            lines.append("  %06X  <no instruction: undecoded bytes>" % a)
            break
        if a in seen_insn:
            lines.append("  ...      (continues at %06X, listed elsewhere)" % a)
            break
        seen_insn.add(a)
        c = ins.getComment(0)
        lines.append("  %06X  %-40s%s" % (a, ins.toString(), ("  ; " + c) if c else ""))
        mn = ins.getMnemonicString().lower()
        for ref in ins.getReferencesFrom():
            if not ref.getReferenceType().isFlow():
                continue
            t = ref.getToAddress().getOffset()
            if t > ROM_END:
                continue
            is_call = ref.getReferenceType().isCall()
            d = depth + 1 if is_call or mn.startswith("jmp") else depth
            if is_call and t not in called:
                called[t] = name_at(t)
            if d <= max_depth:
                queue.append((t, d))
        txt = ins.toString()
        if mn in ("jsr", "jmp") and "[" in txt and ",PC]" in txt:
            # memory-indirect through a PC-relative pointer table:
            # jsr ([disp,PC],Dn.w*4): table = insn + 2 + disp
            import re
            mm = re.search(r"\[(-?0x[0-9a-fA-F]+|-?\d+),PC\]", txt)
            if mm:
                tbl = a + 2 + int(mm.group(1), 0)
                ents = []
                for k in range(64):
                    v = mem.getInt(sp.getAddress(tbl + 4 * k)) & 0xFFFFFFFF
                    if v > ROM_END or v & 1 or v < 0x400:
                        break
                    ents.append(v)
                lines.append("           ; jump table %06X: %s" % (tbl, " ".join("[%d]=%06X" % (k, v) for k, v in enumerate(ents))))
                for v in ents:
                    if v not in called:
                        called[v] = name_at(v) + " (via table %06X)" % tbl
                    if depth + 1 <= max_depth:
                        queue.append((v, depth + 1))
        ft = ins.getFallThrough()
        if ft is None:
            break
        a = ft.getOffset()
        if st.getPrimarySymbol(ft) is not None and a not in seen_insn and a != start:
            # split at labels so branch targets start their own block
            queue.insert(0, (a, depth))
            lines.append("  ...      (falls through to %06X)" % a)
            break
    blocks.append((start, lines))
with open(args[0], "w") as out:
    out.write("; bundle for %06X (depth %d)\n; calls: %s\n" % (root, max_depth, ", ".join(
        "%06X %s" % (k, v) for k, v in sorted(called.items()))))
    for start, lines in sorted(blocks):
        out.write("\n%s:\n" % name_at(start))
        out.write("\n".join(lines) + "\n")
print("export_bundle: %d blocks -> %s" % (len(blocks), args[0]))

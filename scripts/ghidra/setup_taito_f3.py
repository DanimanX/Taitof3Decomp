# Taito F3 (MC68EC020) Ghidra bootstrap: memory map, MMIO labels, vectors.
# @category Taitof3Decomp
# @runtime PyGhidra
#
# Ground truth: MAME 0.289 taito_f3.cpp, taito_f3_state::f3_map (work/vendor).
# Run as a -preScript (before auto-analysis) via:
#   support/pyghidraRun -H <projdir> <projname> -import work/maincpu.bin \
#       -loader BinaryLoader -loader-baseAddr 0 -processor 68000:BE:32:MC68020 \
#       -scriptPath scripts/ghidra -preScript setup_taito_f3.py
# Idempotent: existing blocks/labels/functions are left as-is.

from ghidra.program.model.symbol import SourceType
from ghidra.program.model.data import DWordDataType, PointerDataType, WordDataType

prog = currentProgram  # noqa: F821  (provided by GhidraScript)
mem = prog.getMemory()
st = prog.getSymbolTable()
listing = prog.getListing()
fm = prog.getFunctionManager()
space = prog.getAddressFactory().getDefaultAddressSpace()


def A(x):
    return space.getAddress(x)


# name, start, end_inclusive, read, write, volatile, comment
BLOCKS = [
    ("SOUND_BANK",    0x300000, 0x30007F, False, True,  True,  "sound_bankswitch_w (kirameki-style games)"),
    ("WORK_RAM",      0x400000, 0x41FFFF, True,  True,  False, "128 KiB main work RAM"),
    ("PALETTE_RAM",   0x440000, 0x447FFF, True,  True,  True,  "palette_24bit_w, 8192 x 32-bit"),
    ("F3_CTRL_IO",    0x4A0000, 0x4A001F, True,  True,  True,  "f3_control_r/w: inputs, watchdog, coins, EEPROM"),
    ("F3_TIMER_CTRL", 0x4C0000, 0x4C0003, False, True,  True,  "f3_timer_control_w (IRQ5 pseudo-HBlank)"),
    ("SPRITE_RAM",    0x600000, 0x60FFFF, True,  True,  True,  "spriteram"),
    ("PF_RAM",        0x610000, 0x61BFFF, True,  True,  True,  "playfield tilemap RAM"),
    ("TEXT_RAM",      0x61C000, 0x61DFFF, True,  True,  True,  "text layer RAM"),
    ("CHAR_RAM",      0x61E000, 0x61FFFF, True,  True,  True,  "dynamic char RAM"),
    ("LINE_RAM",      0x620000, 0x62FFFF, True,  True,  True,  "per-line raster effects"),
    ("PIVOT_RAM",     0x630000, 0x63FFFF, True,  True,  True,  "pivot layer RAM"),
    ("VIDEO_CTRL_0",  0x660000, 0x66000F, False, True,  True,  "control_0_w (scroll)"),
    ("VIDEO_CTRL_1",  0x660010, 0x66001F, False, True,  True,  "control_1_w"),
    ("SOUND_DPRAM",   0xC00000, 0xC007FF, True,  True,  True,  "taito_en MB8421 DPRAM (left port)"),
    ("SOUND_RESET_0", 0xC80000, 0xC80003, False, True,  True,  "sound_reset_0_w (CLEAR reset)"),
    ("SOUND_RESET_1", 0xC80100, 0xC80103, False, True,  True,  "sound_reset_1_w (ASSERT reset)"),
]

# MMIO labels: (name, addr, dtype)
LABELS = [
    ("REG_IN0_WATCHDOG",  0x4A0000, DWordDataType()),
    ("REG_IN1_COIN_CTRL", 0x4A0004, DWordDataType()),
    ("REG_IN2",           0x4A0008, DWordDataType()),
    ("REG_IN3",           0x4A000C, DWordDataType()),
    ("REG_IN4_EEPROM",    0x4A0010, DWordDataType()),
    ("REG_IN5_COIN_CTRL2", 0x4A0014, DWordDataType()),
    ("REG_TIMER_CTRL",    0x4C0000, DWordDataType()),
    ("REG_SOUND_RESET_0", 0xC80000, DWordDataType()),
    ("REG_SOUND_RESET_1", 0xC80100, DWordDataType()),
]
for i in range(8):
    LABELS.append(("REG_VIDEO_CTRL0_%d" % i, 0x660000 + 2 * i, WordDataType()))
    LABELS.append(("REG_VIDEO_CTRL1_%d" % i, 0x660010 + 2 * i, WordDataType()))

# M68K exception vectors worth naming (offset -> name)
VEC_NAMES = {
    0x04: "Reset",
    0x08: "Exc_BusError", 0x0C: "Exc_AddressError", 0x10: "Exc_IllegalInsn",
    0x14: "Exc_ZeroDivide", 0x18: "Exc_CHK", 0x1C: "Exc_TRAPV",
    0x20: "Exc_Privilege", 0x24: "Exc_Trace", 0x28: "Exc_LineA", 0x2C: "Exc_LineF",
    0x60: "Exc_Spurious",
    0x64: "IRQ1", 0x68: "IRQ2_VBlank", 0x6C: "IRQ3_VideoSync", 0x70: "IRQ4",
    0x74: "IRQ5_Timer", 0x78: "IRQ6", 0x7C: "IRQ7_NMI",
}
for n in range(16):
    VEC_NAMES[0x80 + 4 * n] = "TRAP_%d" % n

ROM_END = 0x1FFFFF


def make_blocks():
    for name, start, end, r, w, vol, cmt in BLOCKS:
        if mem.getBlock(A(start)) is not None:
            continue
        b = mem.createUninitializedBlock(name, A(start), end - start + 1, False)
        b.setRead(r)
        b.setWrite(w)
        b.setExecute(False)
        b.setVolatile(vol)
        b.setComment(cmt)
        print("block %-14s %06X-%06X" % (name, start, end))
    # WORK_RAM mirror (MAME .mirror(0x20000))
    if mem.getBlock(A(0x420000)) is None:
        b = mem.createByteMappedBlock("WORK_RAM_MIRROR", A(0x420000), A(0x400000), 0x20000, False)
        b.setRead(True)
        b.setWrite(True)
        b.setComment("mirror of WORK_RAM")
        print("block WORK_RAM_MIRROR 420000-43FFFF -> 400000")
    rom = mem.getBlock(A(0))
    if rom is not None:
        rom.setName("ROM")
        rom.setWrite(False)
        rom.setExecute(True)


def put_label(addr, name, dtype=None):
    a = A(addr)
    st.createLabel(a, name, SourceType.IMPORTED).setPrimary()
    if dtype is not None:
        try:
            listing.clearCodeUnits(a, a.add(dtype.getLength() - 1), False)
            listing.createData(a, dtype)
        except Exception as e:  # noqa: BLE001
            print("warn: data at %06X: %s" % (addr, e))


def make_vectors():
    ptr = PointerDataType()
    for off in range(0, 0x100, 4):
        a = A(off)
        listing.clearCodeUnits(a, a.add(3), False)
        if off == 0:
            listing.createData(a, DWordDataType())
            st.createLabel(a, "VEC_InitialSSP", SourceType.IMPORTED)
            continue
        listing.createData(a, ptr)
        target = mem.getInt(a) & 0xFFFFFFFF
        vname = VEC_NAMES.get(off)
        st.createLabel(a, "VEC_" + (vname or "%02X" % off), SourceType.IMPORTED)
        if vname is None or target in (0, 0xFFFFFFFF):
            continue
        target &= 0x00FFFFFF  # 24-bit bus
        if target > ROM_END or target & 1:
            print("vector %02X %s -> %06X outside ROM/odd, skipped" % (off, vname, target))
            continue
        ta = A(target)
        f = fm.getFunctionAt(ta)
        if f is None:
            disassemble(ta)  # noqa: F821
            f = createFunction(ta, vname)  # noqa: F821
        if f is not None:
            existing = f.getName()
            if existing.startswith("FUN_") or existing == vname:
                f.setName(vname, SourceType.IMPORTED)
            else:
                # shared handler (e.g. one stub for many vectors)
                st.createLabel(ta, vname, SourceType.IMPORTED)
            print("vector %02X %-16s -> %06X  fn=%s" % (off, vname, target, f.getName()))


# dariusg: Reset does `lea $408000,A5` (ROM 0x010004) and A5 stays a global
# small-data base for the whole program (IRQ handlers use A5-relative vars).
A5_BASE = 0x408000


def set_a5():
    import java.math.BigInteger as BigInteger
    ctx = prog.getProgramContext()
    reg = ctx.getRegister("A5")
    ctx.setValue(reg, A(0), A(ROM_END), BigInteger.valueOf(A5_BASE))
    st.createLabel(A(A5_BASE), "A5_BASE", SourceType.IMPORTED)
    print("context A5=%06X over ROM" % A5_BASE)


make_blocks()
set_a5()
for nm, addr, dt in LABELS:
    put_label(addr, nm, dt)
for name, start, _e, _r, _w, _v, _c in BLOCKS:
    st.createLabel(A(start), name + "_base", SourceType.IMPORTED)
make_vectors()
print("setup_taito_f3: done")

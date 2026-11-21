# Create structs from symbols/<game>_types.tsv, apply them to data and to
# register parameters (custom storage), so the decompiler shows field names.
# @category Taitof3Decomp
# @runtime PyGhidra
import os
from collections import OrderedDict
from ghidra.program.model.data import (StructureDataType, CategoryPath, ByteDataType,
    SignedByteDataType, WordDataType, ShortDataType, DWordDataType, IntegerDataType,
    PointerDataType, ArrayDataType, DataTypeConflictHandler)
from ghidra.program.model.symbol import SourceType
from ghidra.program.model.listing import ParameterImpl, Function
from ghidra.program.model.listing.Function import FunctionUpdateType

prog = currentProgram  # noqa: F821
args = getScriptArgs()  # noqa: F821
root = os.environ.get("TAITOF3_ROOT", "/PROCESOS/Taitof3Decomp")
path = args[0] if args else os.path.join(root, "symbols/dariusg_types.tsv")
dtm = prog.getDataTypeManager()
cat = CategoryPath("/Taitof3")
sp = prog.getAddressFactory().getDefaultAddressSpace()
BASIC = {"u8": ByteDataType(), "s8": SignedByteDataType(), "u16": WordDataType(),
         "s16": ShortDataType(), "u32": DWordDataType(), "s32": IntegerDataType(),
         "fix16": IntegerDataType(), "ptr": PointerDataType()}
structs, sizes, data_apps, params = OrderedDict(), {}, [], []
for line in open(path):
    if not line.strip() or line.startswith("#"):
        continue
    p = line.rstrip("\n").split("\t")
    if p[0] == "@data":
        data_apps.append((int(p[1], 16), p[2], int(p[3])))
    elif p[0] == "@param":
        params.append((int(p[1], 16), p[2], p[3], p[4]))
    else:
        structs.setdefault(p[0], []).append((int(p[2], 16), p[3], p[4], p[5] if len(p) > 5 else ""))
        sizes[p[0]] = int(p[1], 16)

created = {}
for name, fields in structs.items():
    s = StructureDataType(cat, name, sizes[name])
    for off, typ, fname, cmt in fields:
        dt = BASIC[typ]
        s.replaceAtOffset(off, dt, dt.getLength(), fname, cmt or None)
    created[name] = dtm.addDataType(s, DataTypeConflictHandler.REPLACE_HANDLER)
    print("struct %s (0x%X)" % (name, sizes[name]))


def resolve(typ):
    if typ.endswith("*"):
        return PointerDataType(created[typ[:-1]])
    return created.get(typ) or BASIC[typ]


listing = prog.getListing()
for addr, typ, count in data_apps:
    a = sp.getAddress(addr)
    dt = created[typ] if count == 1 else ArrayDataType(created[typ], count, created[typ].getLength())
    listing.clearCodeUnits(a, a.add(dt.getLength() - 1), False)
    listing.createData(a, dt)
    print("data %06X %s[%d]" % (addr, typ, count))

byfunc = OrderedDict()
for faddr, reg, typ, pname in params:
    byfunc.setdefault(faddr, []).append((reg, typ, pname))
fm = prog.getFunctionManager()
for faddr, plist in byfunc.items():
    f = fm.getFunctionAt(sp.getAddress(faddr))
    if f is None:
        print("warn: no function at %06X" % faddr)
        continue
    ps = [ParameterImpl(n, resolve(t), prog.getRegister(r), prog, SourceType.USER_DEFINED)
          for r, t, n in plist]
    f.setCustomVariableStorage(True)
    f.updateFunction(None, None, FunctionUpdateType.CUSTOM_STORAGE, True, SourceType.USER_DEFINED, ps)
    print("params %s(%s)" % (f.getName(), ", ".join("%s %s@%s" % (t, n, r) for r, t, n in plist)))

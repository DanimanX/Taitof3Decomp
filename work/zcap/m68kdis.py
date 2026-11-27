import sys, capstone
from capstone import m68k
d=open('/PROCESOS/Taitof3Decomp/work/maincpu.bin','rb').read()
a=int(sys.argv[1],16); n=int(sys.argv[2],16) if len(sys.argv)>2 else 0x40
md=capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN|capstone.CS_MODE_M68K_020)
md.detail=True
# capstone prints (d8,An,Xn) without the 68020 scale: annotate it
for i in md.disasm(d[a:a+n], a):
    note=''
    try:
        for op in i.operands:
            if op.type==m68k.M68K_OP_MEM and op.mem.index_reg and op.mem.scale>1:
                note='   ; index *%d'%op.mem.scale
    except Exception: pass
    print('  %06X  %-8s %s%s'%(i.address,i.mnemonic,i.op_str,note))

import sys, capstone
d=open('/PROCESOS/Taitof3Decomp/work/maincpu.bin','rb').read()
a=int(sys.argv[1],16); n=int(sys.argv[2],16) if len(sys.argv)>2 else 0x40
md=capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN|capstone.CS_MODE_M68K_020)
for i in md.disasm(d[a:a+n], a): print('  %06X  %-8s %s'%(i.address,i.mnemonic,i.op_str))

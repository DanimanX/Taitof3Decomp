/* Register-level routines drafted from the disassembly (scripts/strata_fn.py)
 * and accepted bit-exact against MAME captures in tests/test_fn.c. Each takes
 * the 68k register file and leaves it as the ROM does at rts; a return < 0
 * means the call reached a path that is not decompiled yet. */
#ifndef FN_AUTO_H
#define FN_AUTO_H
#include "../common/f3mem.h"
#ifndef F3REGS_T_DEFINED
#define F3REGS_T_DEFINED
typedef struct { uint32_t d[8], a[8]; } f3regs_t;
#endif
/* BEGIN generated */
int Fn_1379CC(const f3mem_t *m, f3regs_t *r); /* ROM 0x1379CC */
int Fn_15F952(const f3mem_t *m, f3regs_t *r); /* ROM 0x15F952 */
int Fn_15F766(const f3mem_t *m, f3regs_t *r); /* ROM 0x15F766 */
int Fn_17B19E(const f3mem_t *m, f3regs_t *r); /* ROM 0x17B19E */
int Fn_1633C4(const f3mem_t *m, f3regs_t *r); /* ROM 0x1633C4 */
int Fn_1333C2(const f3mem_t *m, f3regs_t *r); /* ROM 0x1333C2 */
int Fn_138382(const f3mem_t *m, f3regs_t *r); /* ROM 0x138382 */
int Fn_13C9F8(const f3mem_t *m, f3regs_t *r); /* ROM 0x13C9F8 */
int Fn_13A796(const f3mem_t *m, f3regs_t *r); /* ROM 0x13A796 */
int Fn_13B8A6(const f3mem_t *m, f3regs_t *r); /* ROM 0x13B8A6 */
int Fn_193F22(const f3mem_t *m, f3regs_t *r); /* ROM 0x193F22 */
int Fn_11CDD8(const f3mem_t *m, f3regs_t *r); /* ROM 0x11CDD8 */
int Fn_11F8CA(const f3mem_t *m, f3regs_t *r); /* ROM 0x11F8CA */
int Fn_19A1A0(const f3mem_t *m, f3regs_t *r); /* ROM 0x19A1A0 */
int Fn_181556(const f3mem_t *m, f3regs_t *r); /* ROM 0x181556 */
int Fn_18E196(const f3mem_t *m, f3regs_t *r); /* ROM 0x18E196 */
int Fn_137A38(const f3mem_t *m, f3regs_t *r); /* ROM 0x137A38 */
int Fn_19A1B0(const f3mem_t *m, f3regs_t *r); /* ROM 0x19A1B0 */
int Fn_133046(const f3mem_t *m, f3regs_t *r); /* ROM 0x133046 */
int Fn_11CCAA(const f3mem_t *m, f3regs_t *r); /* ROM 0x11CCAA */
int Fn_12477E(const f3mem_t *m, f3regs_t *r); /* ROM 0x12477E */
int Fn_137A92(const f3mem_t *m, f3regs_t *r); /* ROM 0x137A92 */
int Fn_18E7CE(const f3mem_t *m, f3regs_t *r); /* ROM 0x18E7CE */
int Fn_137C8A(const f3mem_t *m, f3regs_t *r); /* ROM 0x137C8A */
int Fn_137B9C(const f3mem_t *m, f3regs_t *r); /* ROM 0x137B9C */
int Fn_11F868(const f3mem_t *m, f3regs_t *r); /* ROM 0x11F868 */
int Fn_11F8AE(const f3mem_t *m, f3regs_t *r); /* ROM 0x11F8AE */
int Fn_137D5A(const f3mem_t *m, f3regs_t *r); /* ROM 0x137D5A */
int Fn_137D3E(const f3mem_t *m, f3regs_t *r); /* ROM 0x137D3E */
int Fn_13794C(const f3mem_t *m, f3regs_t *r); /* ROM 0x13794C */
int Fn_11BA46(const f3mem_t *m, f3regs_t *r); /* ROM 0x11BA46 */
int Fn_137542(const f3mem_t *m, f3regs_t *r); /* ROM 0x137542 */
int Fn_1330D0(const f3mem_t *m, f3regs_t *r); /* ROM 0x1330D0 */
int Fn_121B60(const f3mem_t *m, f3regs_t *r); /* ROM 0x121B60 */
int Fn_1332BA(const f3mem_t *m, f3regs_t *r); /* ROM 0x1332BA */
int Fn_196BE8(const f3mem_t *m, f3regs_t *r); /* ROM 0x196BE8 */
int Fn_132F06(const f3mem_t *m, f3regs_t *r); /* ROM 0x132F06 */
int Fn_137B80(const f3mem_t *m, f3regs_t *r); /* ROM 0x137B80 */
int Fn_17A85C(const f3mem_t *m, f3regs_t *r); /* ROM 0x17A85C */
int Fn_1292A8(const f3mem_t *m, f3regs_t *r); /* ROM 0x1292A8 */
int Fn_13334E(const f3mem_t *m, f3regs_t *r); /* ROM 0x13334E */
int Fn_133240(const f3mem_t *m, f3regs_t *r); /* ROM 0x133240 */
int Fn_1226E6(const f3mem_t *m, f3regs_t *r); /* ROM 0x1226E6 */
int Fn_121AEE(const f3mem_t *m, f3regs_t *r); /* ROM 0x121AEE */
int Fn_137BF2(const f3mem_t *m, f3regs_t *r); /* ROM 0x137BF2 */
int Fn_16374A(const f3mem_t *m, f3regs_t *r); /* ROM 0x16374A */
int Fn_121C60(const f3mem_t *m, f3regs_t *r); /* ROM 0x121C60 */
int Fn_133370(const f3mem_t *m, f3regs_t *r); /* ROM 0x133370 */
int Fn_1378F6(const f3mem_t *m, f3regs_t *r); /* ROM 0x1378F6 */
int Fn_10C2F4(const f3mem_t *m, f3regs_t *r); /* ROM 0x10C2F4 */
int Fn_12B624(const f3mem_t *m, f3regs_t *r); /* ROM 0x12B624 */
int Fn_12B236(const f3mem_t *m, f3regs_t *r); /* ROM 0x12B236 */
int Fn_133326(const f3mem_t *m, f3regs_t *r); /* ROM 0x133326 */
int Fn_133218(const f3mem_t *m, f3regs_t *r); /* ROM 0x133218 */
int Fn_11F884(const f3mem_t *m, f3regs_t *r); /* ROM 0x11F884 */
int Fn_132F9C(const f3mem_t *m, f3regs_t *r); /* ROM 0x132F9C */
int Fn_138470(const f3mem_t *m, f3regs_t *r); /* ROM 0x138470 */
int Fn_122E50(const f3mem_t *m, f3regs_t *r); /* ROM 0x122E50 */
int Fn_11320A(const f3mem_t *m, f3regs_t *r); /* ROM 0x11320A */
int Fn_110C9C(const f3mem_t *m, f3regs_t *r); /* ROM 0x110C9C */
int Fn_110EC6(const f3mem_t *m, f3regs_t *r); /* ROM 0x110EC6 */
int Fn_10D18A(const f3mem_t *m, f3regs_t *r); /* ROM 0x10D18A */
int Fn_10C548(const f3mem_t *m, f3regs_t *r); /* ROM 0x10C548 */
int Fn_10F41A(const f3mem_t *m, f3regs_t *r); /* ROM 0x10F41A */
int Fn_12B7D4(const f3mem_t *m, f3regs_t *r); /* ROM 0x12B7D4 */
int Fn_11D88A(const f3mem_t *m, f3regs_t *r); /* ROM 0x11D88A */
int Fn_181EA0(const f3mem_t *m, f3regs_t *r); /* ROM 0x181EA0 */
int Fn_122208(const f3mem_t *m, f3regs_t *r); /* ROM 0x122208 */
int Fn_123D98(const f3mem_t *m, f3regs_t *r); /* ROM 0x123D98 */
int Fn_12314C(const f3mem_t *m, f3regs_t *r); /* ROM 0x12314C */
int Fn_104790(const f3mem_t *m, f3regs_t *r); /* ROM 0x104790 */
int Fn_10BE6C(const f3mem_t *m, f3regs_t *r); /* ROM 0x10BE6C */
int Fn_1656E2(const f3mem_t *m, f3regs_t *r); /* ROM 0x1656E2 */
int Fn_1384B0(const f3mem_t *m, f3regs_t *r); /* ROM 0x1384B0 */
int Fn_12001C(const f3mem_t *m, f3regs_t *r); /* ROM 0x12001C */
int Fn_13B2B0(const f3mem_t *m, f3regs_t *r); /* ROM 0x13B2B0 */
int Fn_13B3A6(const f3mem_t *m, f3regs_t *r); /* ROM 0x13B3A6 */
/* END generated */
#endif

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
/* END generated */
#endif

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
/* END generated */
#endif

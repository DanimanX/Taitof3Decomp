#ifndef TAITOF3_OBJ_CBF8A_H
#define TAITOF3_OBJ_CBF8A_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_CBF8A_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0CBF8A */

/* Register view of ROM 0x0D5620 for its register-level wrapper: initialize with
 * the caller's D0/D1/A1, then read back what the ROM leaves at 0x0D5666. */
typedef struct { uint32_t d0, d1, a1; } Obj_CBF8A_D5620Regs;
void Obj_CBF8A_0D5620(const f3mem_t *m, uint32_t obj, Obj_CBF8A_D5620Regs *regs);
#endif

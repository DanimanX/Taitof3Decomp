#ifndef TAITOF3_OBJ_124E30_H
#define TAITOF3_OBJ_124E30_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_124E30_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x124E30 */
/* ROM 0x0CC33A: aim from the object at (A5+0x1D28) toward object a0, with
 * D0w/D1w as offsets; returns 0x80 when there is no target. *out_a1 = the A1
 * the ROM leaves behind (0xFFFFFFFF in the no-target case). */
uint16_t Aim_0CC33A(const f3mem_t *m, uint32_t a0, uint16_t d0w, uint16_t d1w,
                    uint32_t *out_a1);
#endif

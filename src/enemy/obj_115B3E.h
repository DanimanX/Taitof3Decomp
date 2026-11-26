#ifndef TAITOF3_OBJ_115B3E_H
#define TAITOF3_OBJ_115B3E_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_115B3E_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x115B3E */
/* ROM 0x0CC2BC: probability gate. Returns (A5+0x878) % max(1, scaled D0w)
 * (divu then swap: the remainder); callers test it for zero. */
uint16_t Rand_0CC2BC(const f3mem_t *m, uint16_t d0w);
#endif

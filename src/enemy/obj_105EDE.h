#ifndef TAITOF3_OBJ_105EDE_H
#define TAITOF3_OBJ_105EDE_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_105EDE_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x105EDE */
/* ROM 0x1065B6: state pick; returns -1 when the 0x0D0E3A branch was not
 * taken, else the word 0x0D0E3A left in D0. */
int sub_1065B6(const f3mem_t *m, uint32_t obj);
/* Same, with the register file: *d0 is the caller's D0 and receives the word
 * the ROM leaves (0x1065C2 `move.w (0x14,A0),D0w` survives both rts paths
 * that skip 0x0D0E3A, the call overwrites it on the other two). */
int sub_1065B6D0(const f3mem_t *m, uint32_t obj, uint32_t *d0);
#endif

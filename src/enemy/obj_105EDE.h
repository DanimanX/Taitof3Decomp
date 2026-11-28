#ifndef TAITOF3_OBJ_105EDE_H
#define TAITOF3_OBJ_105EDE_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_105EDE_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x105EDE */
/* ROM 0x1065B6: state pick; returns -1 when D0 is untouched, else the word
 * 0x0D0E3A left in D0. */
int sub_1065B6(const f3mem_t *m, uint32_t obj);
#endif

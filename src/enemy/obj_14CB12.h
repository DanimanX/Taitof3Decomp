#ifndef TAITOF3_OBJ_14CB12_H
#define TAITOF3_OBJ_14CB12_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_14CB12_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x14CB12 */
#endif

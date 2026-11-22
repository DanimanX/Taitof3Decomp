#ifndef TAITOF3_OBJ_179628_H
#define TAITOF3_OBJ_179628_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied).
 * 0x179628 is also the frame-script tick (op 12) of the 0x179A5C table. */
int Obj_179628_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x179628 */
#endif

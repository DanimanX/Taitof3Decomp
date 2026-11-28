#ifndef TAITOF3_OBJ_17C700_H
#define TAITOF3_OBJ_17C700_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 1 when the 0x001282 draw ran, 0 when
 * the object was freed at 0x17C78E (+0x1F or +0x1E reached 0xF0), and -1 when
 * it reaches a ROM path not decompiled yet. */
int Obj_17C700_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x17C700 */
#endif

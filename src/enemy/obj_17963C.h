#ifndef TAITOF3_OBJ_17963C_H
#define TAITOF3_OBJ_17963C_H
#include "../common/f3mem.h"
/* Frame-script tick, op 14 of the table at 0x179A5C (init 0x17961E): a
 * 16-byte directional record {spr.l, 1c.w, 2c.l, 30.l, dx.b, dy.b} picked by
 * the angle byte at +0x20. The engine around it, its sibling ops 12 and 15
 * and the object-side tail live in obj_179628.c. */
int Obj_17963C_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x17963C */
#endif

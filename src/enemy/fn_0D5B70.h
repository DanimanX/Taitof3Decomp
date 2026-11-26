#ifndef TAITOF3_ENEMY_FN_0D5B70_H
#define TAITOF3_ENEMY_FN_0D5B70_H
#include "../common/f3mem.h"
/* ROM 0x0D5B70: sprite variant stepper. Decays the 16.16 value at +0x3A by
 * a quarter of +0x58 and then by (value - 0x18000)/16, clamps it to 0..3 and
 * swaps in frame table 0x0D5964[clamp]. Pure Work RAM, no callees. */
void Obj_0D5B70_Step(const f3mem_t *m, uint32_t obj);
#endif

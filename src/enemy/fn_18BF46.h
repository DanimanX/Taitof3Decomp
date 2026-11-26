#ifndef TAITOF3_ENEMY_FN_18BF46_H
#define TAITOF3_ENEMY_FN_18BF46_H
#include "../common/f3mem.h"
/* ROM 0x18BF46: object state step. A5+0x4B7C bits 3 / 2 select a body
 * (0x18BF6E / 0x18BF98, or none); with neither set a negative +0x3E timer is
 * cleared. Returns 0, or -1 on a 0x179A5C dispatch that is not translated. */
int Obj_18BF46_Step(const f3mem_t *m, uint32_t obj);
#endif

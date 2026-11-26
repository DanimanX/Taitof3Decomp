#ifndef TAITOF3_ENEMY_OBJ_1316D2_H
#define TAITOF3_ENEMY_OBJ_1316D2_H
#include "../common/f3mem.h"

/* ROM 0x1316D2: phase-1 callback of the 0x13157A spawner, called with A0 = obj
 * via `movea.l (0xC,A0),A1 ; jsr (A1)`. Returns 0. */
int Obj_1316D2_Callback(const f3mem_t *m, uint32_t obj); /* ROM 0x1316D2 */

#endif

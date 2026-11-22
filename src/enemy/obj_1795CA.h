#ifndef TAITOF3_OBJ_1795CA_H
#define TAITOF3_OBJ_1795CA_H
#include "../common/f3mem.h"
/* Frame-script op 13 (anim-table tick 0x1795CA): read one 18-byte frame
 * record through +0x3A (timer, sprite def, flip, hit box, signed x/y offset)
 * and apply it to the object. Called as the tick of animation op 13. */
int Obj_1795CA_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x1795CA */
#endif

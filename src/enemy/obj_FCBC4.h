#ifndef TAITOF3_OBJ_FCBC4_H
#define TAITOF3_OBJ_FCBC4_H
#include "../common/f3mem.h"
/* Object tick routines (A0 = obj): variants 1..3 of the 0x0FB526 init. */
int Obj_FCBC4_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0FCBC4 */
int Obj_FCBCE_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0FCBCE */
int Obj_FCBD8_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0FCBD8 */
#endif

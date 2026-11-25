#ifndef TAITOF3_OBJ_FB526_H
#define TAITOF3_OBJ_FB526_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_FB526_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0FB526 */
/* ROM 0x0FB52A: the init body after +0x10 is set (A0 = obj). */
int Obj_FB52A_Body(const f3mem_t *m, uint32_t obj);
#endif

#ifndef TAITOF3_OBJ_17BE1C_H
#define TAITOF3_OBJ_17BE1C_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_17BE1C_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x17BE1C */
/* ROM 0x17C704: out-of-HP hand-off (reward via 0x17B02E, then the death path). */
int Obj_17C704_Body(const f3mem_t *m, uint32_t obj);
#endif

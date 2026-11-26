#ifndef TAITOF3_OBJ_17C908_H
#define TAITOF3_OBJ_17C908_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_17C908_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x17C908 */
/* ROM 0x17C97A: follow the parent at +0x16/+0x1A, animation tick, draw,
 * arm 0x17C9D0 (-1 from an untranslated 0x179A5C index). */
int Obj_17C97A_Body(const f3mem_t *m, uint32_t obj);
#endif

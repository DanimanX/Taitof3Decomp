#ifndef TAITOF3_OBJ_11949A_H
#define TAITOF3_OBJ_11949A_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_11949A_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x11949A */
void Text_TypeChar(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x119590: +0x10 = a1, draw the string at +0x22 at text cell (+0x14,+0x18) colour +0x1C, +0x14 += 1 */
#endif

#ifndef TAITOF3_OBJ_126FF6_H
#define TAITOF3_OBJ_126FF6_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_126FF6_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x126FF6 */

/* ROM 0x11C096: the shared 0x11B/0x11C hit spark. Returns -1 when it reaches
 * the untranslated hit-record builder at 0x11C106. */
int Obj_126FF6_HitSpark(const f3mem_t *m, uint32_t obj);
#endif

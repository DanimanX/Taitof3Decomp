#ifndef TAITOF3_OBJSCRIPT_CF4B8_H
#define TAITOF3_OBJSCRIPT_CF4B8_H
#include "../common/f3mem.h"
/* Per-frame movement step: dispatches on the current movement mode
 * obj+0x20 (set by the script interpreter ObjScript_CF1E4), table 0xCF4C8,
 * 29 modes (0..11 do nothing). a1 = caller's A1: {dx.w, dy.w} aim offset
 * used by mode 15 (home on the nearest live player). Returns 0, or -1 at a
 * path not decompiled yet. */
int ObjScript_CF4B8(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0xCF4B8 */
#endif

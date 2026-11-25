#ifndef TAITOF3_OBJ_130312_H
#define TAITOF3_OBJ_130312_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_130312_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x130312 */
/* Same tick with the caller's D3: bits 7..0 of the spawn argument are never
 * written by the routine, so they carry whatever D3 held at the pool call
 * (the pool runner does not set D3; it is the previous tick's leftover). */
int Obj_130312_TickD3(const f3mem_t *m, uint32_t obj, uint32_t d3);
#endif

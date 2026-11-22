#ifndef TAITOF3_OBJ_1194C2_H
#define TAITOF3_OBJ_1194C2_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_1194C2_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x1194C2 */
uint32_t Text_DrawString(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t col); /* ROM 0x005666: string -> text RAM {col*2, char} pairs; returns the end address */
#endif

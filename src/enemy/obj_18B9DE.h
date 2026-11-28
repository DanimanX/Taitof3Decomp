#ifndef TAITOF3_OBJ_18B9DE_H
#define TAITOF3_OBJ_18B9DE_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_18B9DE_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x18B9DE */
/* ROM 0x18BB50: credit + score counters + the 0x179DEE burst. Returns the D0
 * the ROM leaves at its rts: the 0x01057C slot address, or 0xFFFFFFFF when the
 * pool is full. */
uint32_t Obj_18BB50(const f3mem_t *m, uint32_t obj);
/* Same, with the register file: `a1` is the caller's A1 and receives the
 * 0x17B02E credit pointer when that branch is taken (0x18BB5C movea.l D0,A1). */
uint32_t Obj_18BB50R(const f3mem_t *m, uint32_t obj, uint32_t *a1);
#endif

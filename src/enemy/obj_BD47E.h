#ifndef TAITOF3_OBJ_BD47E_H
#define TAITOF3_OBJ_BD47E_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_BD47E_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0BD47E */
/* ROM 0x176832: line-RAM zoom ramp (frame args in ROM push order). */
void sub_176832(const f3mem_t *m, uint32_t a0, uint32_t a1,
                uint16_t d7w, uint16_t d6w, uint32_t v4298,
                uint32_t d5, uint32_t d3, uint16_t v42a0);
/* ROM 0x1768CA: line-RAM scroll ramp. */
void sub_1768CA(const f3mem_t *m, uint32_t a0, uint16_t d7w, uint16_t d6w,
                uint32_t v4298, uint32_t d3, uint32_t d4);
#endif

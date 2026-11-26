#ifndef TAITOF3_OBJ_CBF04_H
#define TAITOF3_OBJ_CBF04_H
#include "../common/f3mem.h"
/* ROM 0x0CBEEA: shared tail reached by jmp with D1.w = child count and
 * A1 = child routine; latches both and falls into 0x0CBF04. */
int Obj_CBEEA(const f3mem_t *m, uint32_t obj, uint16_t d1w, uint32_t a1);
/* ROM 0x0CBF04: wait for A5+0x1D28, then spawn the +0x22 children. */
int Obj_CBF04_Tick(const f3mem_t *m, uint32_t obj);
#endif

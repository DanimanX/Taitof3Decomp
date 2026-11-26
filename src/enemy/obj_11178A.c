#include "obj_11178A.h"
#include "../objects/obj_common.h"

/* ROM 0x11178A: resets the object fields, then rolls two rng values:
 * (rng & 0xff) + 0x20 into +0x14 and (rng & 0x3f) + 0xa8 into +0x18,
 * then stores the 0x1117cc continuation. */
int Obj_11178A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11178A: jsr Obj_ResetFields */
    Obj_ResetFields(m, obj);

    /* 0x111790: movem.l {A0},-(SP) ; jsr Rng_Next ; movem.l (SP)+,{A0} */
    uint32_t r1 = Rng_NextMem(m);
    /* 0x11179E: andi.w #0xff,D0w ; addi.w #0x20,D0w ; move.w D0w,(0x14,A0) */
    uint16_t v1 = (uint16_t)((r1 & 0xffu) + 0x20u);
    wr16(m, obj + 0x14u, v1);

    /* 0x1117AA: movem.l {A0},-(SP) ; jsr Rng_Next ; movem.l (SP)+,{A0} */
    uint32_t r2 = Rng_NextMem(m);
    /* 0x1117B8: andi.w #0x3f,D0w ; addi.w #0xa8,D0w ; move.w D0w,(0x18,A0) */
    uint16_t v2 = (uint16_t)((r2 & 0x3fu) + 0xa8u);
    wr16(m, obj + 0x18u, v2);

    /* 0x1117C4: move.l #0x1117cc,(A0) */
    wr32(m, obj + 0x00u, 0x001117CCu);
    return 0;
}

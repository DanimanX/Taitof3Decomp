#include "obj_17AD56.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"

/* ROM 0x17AD56: the scripted mover that drags its parent along. It mirrors the
 * parent-relative offset +0x50/+0x54 into the screen position, steps the
 * 10-byte movement script at +0x44 (0x178982), and adds the movement that step
 * produced to the parent's own +0x50/+0x54. The latch is 0x17AD4E unless the
 * script pointer has been exhausted to -1, in which case the object is freed. */

/* ROM 0x17AD4E: re-latch this routine */
static void lab_17AD4E(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0017AD56u);
}

int Obj_17AD56_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17AD56: move.w (0x50,A0),(0x14,A0) ; move.w (0x54,A0),(0x18,A0) */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17AD62: jsr 0x178982 -- one pass of the movement script */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x17AD68: the movement this step applied: (0x50,0x54) - (0x14,0x18) */
    {
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
        /* 0x17AD78: movea.l (0x8,A0),A4 ; add.w D0,(0x50,A4) ; add.w D1,(0x54,A4) */
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, a4 + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + d0));
        wr16(m, a4 + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + d1));
    }

    /* 0x17AD84: cmpi.l #-1,(0x44,A0) ; bne.b 0x17AD4E */
    if ((int32_t)rd32(m, obj + 0x44u) != -1) {
        lab_17AD4E(m, obj);
        return 0;
    }

    /* 0x17AD8E: jsr 0x0105EC */
    Obj_Free(m, obj);
    return 0;
}

#include "obj_DE7E0.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690: advance the animation sequence when the frame timer expires */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    int16_t d0;

    wr16(m, obj + 0x4C, (uint16_t)(rd16(m, obj + 0x4C) - 1u));
    if (rd16(m, obj + 0x4C) != 0u)
        return;

    a1 = (uint32_t)((int32_t)rd32(m, obj + 0x48) + 0x0C);
    wr32(m, obj + 0x48, a1);
    d0 = (int16_t)rd16(m, a1);
    a1 += 2u;
    if (d0 >= 0) {
        wr16(m, obj + 0x4C, (uint16_t)d0);
        d0 = (int16_t)rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1C, (uint8_t)d0);
        wr32(m, obj + 0x10, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68, rd32(m, a1));
        return;
    }

    /* loop marker: word 0 = repeat offset, non-zero = rewind, zero = end */
    if (rd16(m, a1) != 0u) {
        a1 = (uint32_t)((int32_t)a1 - (int32_t)(int16_t)rd16(m, a1));
        wr32(m, obj + 0x48, a1);
        wr16(m, obj + 0x4C, rd16(m, a1));
        a1 += 2u;
        d0 = (int16_t)rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1C, (uint8_t)d0);
        wr32(m, obj + 0x10, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68, rd32(m, a1));
    }
}

/* ROM 0x0DE7E0: scroll with the screen, animate, die at end of sequence */
int Obj_DE7E0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}

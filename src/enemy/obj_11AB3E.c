#include "obj_11AB3E.h"
#include "../objects/obj_pool.h"

/* ROM 0x11AB36: timer still positive: stay on 0x11AB3E */
static void sub_11AB36(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011AB3Eu);
}

/* ROM 0x11AB3E: accelerate the global 0x1D20 scroll by the object's dx, then
 * subtract the new dx from 0xFFFFF354; when the +0x6 timer runs out clear the
 * global flags, push +0x34 into 0x1D36, re-darken the row and free */
int Obj_11AB3E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x06u) == 0x0030u) {               /* 0x11AB3E */
        wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x20u)); /* bset.b #5 */
    }

    uint32_t d0 = rd32(m, obj + 0x14u);
    wr32(m, F3_A5 + 0x1D20u, rd32(m, F3_A5 + 0x1D20u) + d0);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x00002000u);   /* addi.l */
    d0 = rd32(m, obj + 0x14u);
    wr32(m, F3_A5 - 0x0CACu, rd32(m, F3_A5 - 0x0CACu) - d0);    /* sub.l */

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_11AB36(m, obj);                              /* 0x11AB6A */
        return 0;
    }

    /* 0x11AB6C: bclr.b #5 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & (uint8_t)~0x20u));
    wr8(m, F3_A5 + 0x1D36u, rd8(m, obj + 0x34u));
    wr32(m, F3_A5 - 0x0CACu, 0u);
    wr32(m, F3_A5 + 0x1D20u, 0u);
    /* 0x11AB80: andi.w #-0x13 */
    wr16(m, F3_A5 - 0x0CA4u, (uint16_t)(rd16(m, F3_A5 - 0x0CA4u) & 0xFFECu));
    wr16(m, obj + 0x06u, 0x0004u);
    /* 0x11AB8C beq.b tests Z from move.w #4 (Z clear) -> fall through */
    wr32(m, obj + 0x00u, 0x0011AB96u);
    return 0;
}

/* (0x11AB9C..0x11ABD4 never run from here: that path (0x11AB96) sets bits
 * 0x55 of the 0x626000 row words and frees the object.) */

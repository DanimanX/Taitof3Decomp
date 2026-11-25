#include "obj_11AAF0.h"

/* ROM 0x11AAF0: countdown tick. While +0x6 is positive it only re-arms itself
 * (0x11AAE8). On the last tick it walks the 0xE8 word pairs of the two line-RAM
 * rows at 0x626000/0x627400, darkening one row and lightening the other, sets
 * bits 1|4 of the global A5-0xCA4, clears the +0x14 accumulator, reloads the
 * timer with 0x50 and switches to 0x11AB3E (the scroll phase). */
int Obj_11AAF0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11AAF0: subq.w #1,(0x6,A0) ; bgt.b 0x11AAE8 */
    int16_t t = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        /* ROM 0x11AAE8: move.l #0x11AAF0,(A0) ; rts */
        wr32(m, obj + 0x00u, 0x0011AAF0u);
        return 0;
    }

    /* 0x11AAF6: lea (0x626000).l,A1 ; adda.w (-0x5D56,A5),A1 */
    uint32_t a1 = 0x00626000u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    /* 0x11AB00: lea (0x627400).l,A2 ; adda.w (-0x5D56,A5),A2 */
    uint32_t a2 = 0x00627400u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    /* 0x11AB0A: move.w #0xE7,D0w ; 0x11AB0E dbf loop over 0xE8 word pairs */
    for (int32_t n = 0xE7; n >= 0; n--) {
        uint16_t d1 = rd16(m, a1);          /* 0x11AB0E: move.w (A1),D1w */
        uint16_t d2 = rd16(m, a2);          /* 0x11AB10: move.w (A2),D2w */
        d1 = (uint16_t)(d1 & 0xFFAAu);      /* 0x11AB12: andi.w #0xFFAA */
        d2 = (uint16_t)(d2 | 0xF000u);      /* 0x11AB16: ori.w #0xF000 */
        wr16(m, a2, d2);                    /* 0x11AB1A: move.w D2w,(A2)+ */
        wr16(m, a1, d1);                    /* 0x11AB1C: move.w D1w,(A1)+ */
        a2 += 2u;
        a1 += 2u;
    }

    /* 0x11AB22: ori.w #0x12,(-0xCA4,A5) */
    wr16(m, F3_A5 - 0x0CA4u, (uint16_t)(rd16(m, F3_A5 - 0x0CA4u) | 0x0012u));
    /* 0x11AB28: move.l #0,(0x14,A0) */
    wr32(m, obj + 0x14u, 0u);
    /* 0x11AB30: move.w #0x50,(0x6,A0) */
    wr16(m, obj + 0x06u, 0x0050u);

    /* ROM 0x11AB36: move.l #0x11AB3E,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x0011AB3Eu);
    return 0;
}

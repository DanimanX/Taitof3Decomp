/* ROM 0x18A06E: first half of the 0x18A0C2 pair — 1-frame delay that sets the
 * A5+0x4B3A bit 0xD flag, then arms the 1-frame state 0x18A08A. */
#include "obj_18A06E.h"

/* bfset (0x4B3A,A5){0xD:0x1}: bit 0xD of the longword starting at 0x4B3A
 * lands in byte +0x1 with mask 0x80 >> 5 = 0x04. */
#define A5_B3A_D (F3_A5 + 0x4B3Au + 0x1u)

/* The fall-through behind the beq at 0x18A080 (0x18A090 -> spawn 0x18A0CC)
 * is dead: it tests the Z left by `move.w #1,(A0+6)`, always 0. */

/* ROM 0x18A06E */
int Obj_18A06E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A06E: subq.w #1,(A0+6) ; bgt 0x18A066 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x0018A06Eu);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x18A074: bfset (0x4B3A,A5){0xD:0x1} */
    wr8(m, A5_B3A_D, (uint8_t)(rd8(m, A5_B3A_D) | 0x04u));

    /* 0x18A07A: move.w #1,(A0+6) ; move.l #0x18A08A,(A0) */
    wr16(m, obj + 0x06u, 0x0001u);
    wr32(m, obj, 0x0018A08Au);
    return 0;
}

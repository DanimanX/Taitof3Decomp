/* ROM 0x189CCC: first half of the 0x189C56 pair — 1-frame delay that sets the
 * A5+0x4B3A bit 0xD flag, then arms the 1-frame state 0x189CE8. */
#include "obj_189CCC.h"

/* bfset/bfclr (0x4B3A,A5){0xD:0x1}: bit 0xD of the longword starting at
 * 0x4B3A lands in byte +0x1 with mask 0x80 >> 5 = 0x04. */
#define A5_B3A_D (F3_A5 + 0x4B3Au + 0x1u)

/* The fall-through behind the beq at 0x189CDE (which would run 0x189CEE and
 * spawn 0x189D2A) is dead: it tests the Z left by `move.w #1,(A0+6)`, and a
 * move.w leaves Z=0 unconditionally. */

/* ROM 0x189CCC */
int Obj_189CCC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189CCC: subq.w #1,(A0+6) ; bgt 0x189CC4 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x00189CCCu);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x189CD2: bfset (0x4B3A,A5){0xD:0x1} */
    wr8(m, A5_B3A_D, (uint8_t)(rd8(m, A5_B3A_D) | 0x04u));

    /* 0x189CD8: move.w #1,(A0+6) ; move.l #0x189CE8,(A0) */
    wr16(m, obj + 0x06u, 0x0001u);
    wr32(m, obj, 0x00189CE8u);
    return 0;
}

/* ROM 0x18A08A: second half of the 0x18A0C2 pair — clears the A5+0x4B3A bit
 * 0xD flag, spawns the 0x18A0CC actor, then arms the 8-frame state 0x18A0C2. */
#include "obj_18A08A.h"
#include "../objects/obj_pool.h"

/* bfclr (0x4B3A,A5){0xD:0x1}: bit 0xD of the longword starting at 0x4B3A
 * lands in byte +0x1 with mask 0x80 >> 5 = 0x04. */
#define A5_B3A_D (F3_A5 + 0x4B3Au + 0x1u)

/* The fall-through behind the beq at 0x18A0B8 (0x18A0C8 -> 0x189A8E) is dead:
 * it tests the Z left by `move.w #8,(A0+6)`, always 0. */

/* ROM 0x18A08A */
int Obj_18A08A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A08A: subq.w #1,(A0+6) ; bgt 0x18A082 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x0018A08Au);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x18A090: bfclr (0x4B3A,A5){0xD:0x1} */
    wr8(m, A5_B3A_D, (uint8_t)(rd8(m, A5_B3A_D) & (uint8_t)~0x04u));

    /* 0x18A096: pea (0x0).w ; move.w #2,-(SP) ; move.w #3,-(SP) ;
     *           move.l #0x18A0CC,-(SP) ; jsr 0x1057C */
    (void)Obj_Spawn(m, obj, 0x0018A0CCu, 3u, 2u, 0x00000000u);

    /* 0x18A0B2: move.w #8,(A0+6) ; move.l #0x18A0C2,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x0018A0C2u);
    return 0;
}

/* ROM 0x189CE8: second half of the 0x189C56 pair — clears the A5+0x4B3A bit
 * 0xD flag, spawns the 0x189D2A actor, then arms the 8-frame state 0x189D20. */
#include "obj_189CE8.h"
#include "../objects/obj_pool.h"

/* bfclr (0x4B3A,A5){0xD:0x1}: bit 0xD of the longword starting at 0x4B3A
 * lands in byte +0x1 with mask 0x80 >> 5 = 0x04. */
#define A5_B3A_D (F3_A5 + 0x4B3Au + 0x1u)

/* The fall-through behind the beq at 0x189D16 (which would run 0x189D26 ->
 * 0x189A8E) is dead: it tests the Z left by `move.w #8,(A0+6)`, and a move.w
 * leaves Z=0 unconditionally. */

/* ROM 0x189CE8 */
int Obj_189CE8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189CE8: subq.w #1,(A0+6) ; bgt 0x189CE0 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x00189CE8u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x189CEE: bfclr (0x4B3A,A5){0xD:0x1} */
    wr8(m, A5_B3A_D, (uint8_t)(rd8(m, A5_B3A_D) & (uint8_t)~0x04u));

    /* 0x189CF4: pea (0x0).w ; move.w #2,-(SP) ; move.w #3,-(SP) ;
     *           move.l #0x189D2A,-(SP) ; jsr 0x1057C */
    (void)Obj_Spawn(m, obj, 0x00189D2Au, 3u, 2u, 0x00000000u);

    /* 0x189D10: move.w #8,(A0+6) ; move.l #0x189D20,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x00189D20u);
    return 0;
}

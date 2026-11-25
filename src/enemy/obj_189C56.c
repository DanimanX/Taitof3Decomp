/* ROM 0x189C56: timer half of the 0x189C7C pair — re-arms itself while
 * (A0+6) > 1; on expiry it zeroes the palette-slide offsets of the object at
 * A5+0x4B74, re-arms 8 frames and parks on 0x189C7C (0x189A8E is dead here). */
#include "obj_189C56.h"

#define A5_B74   (F3_A5 + 0x4B74u)

/* The fall-through behind the beq at 0x189C72 (0x189C82 -> 0x189A8E) is dead:
 * it tests the Z left by `move.w #8,(A0+6)`, and a move.w leaves Z=0. */

/* ROM 0x189C56 */
int Obj_189C56_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189C56: subq.w #1,(A0+6) ; bgt 0x189C4E (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x00189C56u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x189C5C: movea.l (0x4B74,A5),A1 ; move.w #-0x20,(0x16,A1) ;
     *           move.w #-0x30,(0x1A,A1) */
    uint32_t a1 = rd32(m, A5_B74);
    wr16(m, a1 + 0x16u, 0xFFE0u);
    wr16(m, a1 + 0x1Au, 0xFFD0u);

    /* 0x189C6C: move.w #8,(A0+6) ; move.l #0x189C7C,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x00189C7Cu);
    return 0;
}

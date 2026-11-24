/* ROM 0x189E2E: 0x189FE2's sibling — 0x28-frame delay that clears the
 * 0x611000 block, points A5+0x4B74 at (-0x20,-0x60), then arms the 8-frame
 * re-arm state 0x189E6A. */
#include "obj_189E2E.h"
#include "../objects/obj_common.h"

#define A5_B74   (F3_A5 + 0x4B74u)

/* The fall-through behind the beq at 0x189E60 (0x189E70 -> 0x189A8E) is dead:
 * it tests the Z left by `move.w #8,(A0+6)`, which is always 0. So neither the
 * 0x189A8E state merge nor 0x189AF6 is reachable from this tick. */

/* ROM 0x189E2E */
int Obj_189E2E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189E2E: subq.w #1,(A0+6) ; bgt 0x189E26 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x00189E2Eu);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x189E34: pea (0x611000).l ; move.l #0x81572,-(SP) ; jsr 0x54AC */
    Video_ClearRect(m, 0x00081572u, 0x00611000u);

    /* 0x189E4A: movea.l (0x4B74,A5),A1 ; move.w #-0x20,(0x16,A1) ;
     *           move.w #-0x60,(0x1A,A1) */
    uint32_t a1 = rd32(m, A5_B74);
    wr16(m, a1 + 0x16u, 0xFFE0u);
    wr16(m, a1 + 0x1Au, 0xFFA0u);

    /* 0x189E5A: move.w #8,(A0+6) ; move.l #0x189E6A,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x00189E6Au);
    return 0;
}

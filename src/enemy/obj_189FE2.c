/* ROM 0x189FE2: 0x28-frame delay that clears the 0x611000 block, points
 * A5+0x4B74 at (-0x28,-0x68), then arms the 8-frame re-arm state 0x18A01E. */
#include "obj_189FE2.h"
#include "../objects/obj_common.h"

#define A5_B74   (F3_A5 + 0x4B74u)

/* 0x189AF6 (bfclr/bfset on A5+0x4B26) and 0x189A8E are not reachable from
 * this tick: 0x18A024 sits behind a `beq` that follows the move.w at
 * 0x18A00E and therefore never fires. */

/* ROM 0x189FE2 */
int Obj_189FE2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189FE2: subq.w #1,(A0+6) ; bgt 0x189FDA (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x00189FE2u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x189FE8: pea (0x611000).l ; move.l #0x8159C,-(SP) ; jsr 0x54AC */
    Video_ClearRect(m, 0x0008159Cu, 0x00611000u);

    /* 0x189FFE: movea.l (0x4B74,A5),A1 ; move.w #-0x28,(0x16,A1) ;
     *           move.w #-0x68,(0x1A,A1) */
    uint32_t a1 = rd32(m, A5_B74);
    wr16(m, a1 + 0x16u, 0xFFD8u);
    wr16(m, a1 + 0x1Au, 0xFF98u);

    /* 0x18A00E: move.w #8,(A0+6) ; move.l #0x18A01E,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x0018A01Eu);
    return 0;
}

/* ROM 0x18A1D0: 0x28-frame delay that clears the 0x611000 block, points A5+0x4B74
 * at (-0x30,-0x70), then arms the 8-frame re-arm state 0x18A20C. */
#include "obj_18A1D0.h"
#include "../objects/obj_common.h"

#define A5_B74   (F3_A5 + 0x4B74u)

/* The 0x18A1D6 path calls only 0x54AC and stores two immediates: none of the
 * 0x189AF6 / 0x189A8E state-merge helpers is reachable from 0x18A1D0 (their
 * caller 0x189A8E is entered from 0x18A212, behind a `beq` that follows the
 * move.w at 0x18A1FC and therefore never fires). */

/* ROM 0x18A1D0 */
int Obj_18A1D0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A1D0: subq.w #1,(A0+6) ; bgt 0x18A1C8 (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x0018A1D0u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x18A1D6: pea (0x611000).l ; move.l #0x815E6,-(SP) ; jsr 0x54AC */
    Video_ClearRect(m, 0x000815E6u, 0x00611000u);

    /* 0x18A1EC: movea.l (0x4B74,A5),A1 ; move.w #-0x30,(0x16,A1);
     *           move.w #-0x70,(0x1A,A1) */
    uint32_t a1 = rd32(m, A5_B74);
    wr16(m, a1 + 0x16u, 0xFFD0u);
    wr16(m, a1 + 0x1Au, 0xFF90u);

    /* 0x18A1FC: move.w #8,(A0+6) ; move.l #0x18A20C,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x0018A20Cu);
    return 0;
}

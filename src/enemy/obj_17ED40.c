#include "obj_17ED40.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17ED40: one-shot emitter. Arms +0x4C from the global at A5+0x1D28,
 * places the object on top of its parent, gives up (0x17C78E, free) when the
 * arm count is spent or the object leaves the 0x1849F8 region, otherwise
 * spawns eight 0x17EFC8 children and parks on 0x17EDA4. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17ED40 */
int Obj_17ED40_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17ED40: move.l (0x1D28,A5),(0x4C,A0); ble 0x17C78E */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    if ((int32_t)rd32(m, obj + 0x4Cu) <= 0) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17ED4A: bsr 0x17BD68 */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17ED4E..0x17ED6A: cmp2.w (0x1849F8)/(0x1849FC); bcs 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (!(x > (int16_t)rd16(m, 0x001849F8u) && x < (int16_t)rd16(m, 0x001849FAu)) ||
            !(y > (int16_t)rd16(m, 0x001849FCu) && y < (int16_t)rd16(m, 0x001849FEu))) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17ED6E: moveq #7,D2 ; per iteration
     *   move.w D2w,D1w; ext.l D1; lsl.b #5,D1b   -> arg = (D2 << 5) low byte
     *   move.l D1,-(SP); #2; #9; #0x17EFC8; jsr 0x1057C */
    uint32_t last_spawn = 0u;
    for (uint16_t d2 = 7u; ; d2--) {
        uint32_t d1 = (uint32_t)(uint8_t)((uint8_t)d2 << 5);
        last_spawn = Obj_Spawn(m, obj, 0x0017EFC8u, 9u, 2u, d1);
        if (d2 == 0u)
            break;
    }

    /* 0x17ED94: move.w #2,(0x6,A0); beq 0x17EDAA -- Z left by the last spawn
     * call (D0 = its result), not by the move. The 0x17ED40 capture pair has
     * the spawn succeeding (Z clear) and ends on 0x17EDA4 with +0x06 = 2. */
    wr16(m, obj + 0x06u, 2u);
    if (last_spawn == 0u) {
        lab_17C78E(m, obj);                         /* 0x17EDAA bra 0x17C78E */
        return 0;
    }

    /* 0x17ED9C: move.l #0x17EDA4,(A0) */
    wr32(m, obj + 0x00u, 0x0017EDA4u);
    return 0;
}

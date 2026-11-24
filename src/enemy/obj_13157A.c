#include "obj_13157A.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x13157A: a parent-relative drifting shot. Each tick it drags its
 * position by the parent's +0x14/+0x18 velocity (minus 8), draws, and then
 * advances one of three animation tables depending on the +0x6 phase:
 *   +0x6 <  1: table 0x131672, and a negative first word bumps +0x6 and +0x3A
 *   +0x6 == 1: runs the +0x0C callback and steps table 0x131698
 *   +0x6 >  1: table 0x1316AC, and a negative first word switches to the
 *              follower state 0x131616 (0x131606)
 * Only the first phase appears in the capture, so the other two are left as
 * undecompiled paths. */
int Obj_13157A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u) {
        /* 0x131580 bne.w -> 0x131664, not covered here */
        return -1;
    }

    {
        uint32_t parent = rd32(m, obj + 0x08u);
        /* 0x131588: move.w (0x16,A0),D0 ; add.w (0x14,A1),D0 ; subq.w #8,D0 */
        wr16(m, obj + 0x14u,
             (uint16_t)((uint16_t)(rd16(m, obj + 0x16u) + rd16(m, parent + 0x14u)) - 8u));
        wr16(m, obj + 0x18u,
             (uint16_t)((uint16_t)(rd16(m, obj + 0x1Au) + rd16(m, parent + 0x18u)) - 8u));
    }

    /* 0x1315A4 bsr 0x12FF36: keeps the parked +0x32AA pointer in sync */
    Obj_DrawSprite(m, obj);

    int16_t ph = (int16_t)rd16(m, obj + 0x06u);
    if (ph == 1) {
        /* 0x1315DA: run the callback then step table 0x131698 */
        return -1;
    }
    if (ph > 1) {
        /* 0x1315EE: table 0x1316AC, may switch to 0x131616 */
        return -1;
    }

    /* 0x1315BC: phase 0, table 0x131672 */
    Obj_AnimFrameStep(m, obj, 0x00131672u);
    {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, 0x00131672u + (uint32_t)(int32_t)off) < 0) {
            wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));
            wr16(m, obj + 0x3Au, 0xFFFEu);
        }
    }
    wr32(m, obj + 0x00u, 0x0013157Au); /* 0x131572 */
    return 0;
}

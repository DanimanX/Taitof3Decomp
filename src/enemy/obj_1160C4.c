#include "obj_1160C4.h"
#include "../objects/obj_pool.h"

/* ROM 0x1160C4: spawner. Every 4th tick it drops one child 0x116118 that
 * follows its parent; it ends as soon as the parent (+0x8) is flagged dying
 * (bit4/bit0 of +0x50) or does not carry the spawn bit (bit3). */
int Obj_1160C4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1160C4: A1 = (0x8,A0) -- the parent object */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if ((rd8(m, a1 + 0x50u) & 0x10u) != 0u)
        goto free;
    if ((rd8(m, a1 + 0x50u) & 0x08u) == 0u) {
        if ((rd8(m, a1 + 0x50u) & 0x01u) != 0u)
            goto free;
    }

    /* 0x1160E6 */
    if ((rd16(m, obj + 0x06u) & 0x0003u) == 0u) {
        /* 0x1160F2: pea 0 / move.w #1,-(SP) / move.w #0,-(SP) / move.l #0x116118 */
        Obj_Spawn(m, obj, 0x00116118u, 0u, 1u, 0u);
    }

    /* 0x11610E */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x1160BC: stay on this routine */
        wr32(m, obj + 0x00u, 0x001160C4u);
        return 0;
    }

free:
    /* 0x11622A */
    Obj_Free(m, obj);
    return 0;
}

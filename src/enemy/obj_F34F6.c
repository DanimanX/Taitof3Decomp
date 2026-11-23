#include "obj_F34F6.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0E3A: 16-bit LCG at A5+0x1D60 (n = n*5+1), returns n >> 8 */

/* ROM 0x0F34F6: ring emitter that spawns 0x0F3582 "bubbles" at its own
 * position while obj+0x21 (count) and obj+0x22 (enable) allow. */
int Obj_F34F6_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    /* 0x0F3514 */
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) != 0u)
        return 0;

    /* 0x0F351C */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);
    uint32_t a1 = Obj_Spawn(m, obj, 0x000F3582u, 5, 2, 0u);
    if ((int32_t)a1 < 0) {                       /* tst.l D0; bmi */
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    /* 0x0F3548 */
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    uint16_t d0 = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u);
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + d0));
    wr8(m, a1 + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}

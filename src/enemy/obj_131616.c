#include "obj_131616.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x12FF36: colour override from the boss/current-enemy slot A5+0x32AA */
static void sub_12FF36(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);
    uint8_t d0 = rd8(m, a1 + 0x1Du);
    if (d0 != rd8(m, a1 + 0x4Fu))
        d0 = rd8(m, obj + 0x4Fu);
    wr8(m, obj + 0x1Du, d0);
}

/* ROM 0x131616: the mid-boss core (parented, drawn at the parent, zoomed). */
int Obj_131616_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x131616 */
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x131620: follow the parent (+8); the offset closes as the zoom opens */
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x16u) + rd16(m, a1 + 0x14u));
    uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Au) + rd16(m, a1 + 0x18u));
    int16_t d2w = (int16_t)(((int16_t)(uint16_t)(uint8_t)rd8(m, obj + 0x1Eu) - 0x100) >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(d0w + d2w));
    wr16(m, obj + 0x18u, (uint16_t)(d1w + d2w));

    sub_12FF36(m, obj);                                         /* 0x13164C */
    Obj_DrawSprite(m, obj);                                     /* 0x131650 */

    /* 0x131656: zoom grows by 0x1010 (0x1000 scale) while the object pops
     * out of the parent; at the near limit it is released (0x131664). */
    int32_t z = (int32_t)rd16(m, obj + 0x1Eu) + 0x1010;
    wr16(m, obj + 0x1Eu, (uint16_t)z);
    if (z >= 0xEF00) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj, 0x00131616u);
    return 0;
}

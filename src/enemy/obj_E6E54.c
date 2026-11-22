#include "obj_E6E54.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CFA86: load palette/sprite/ext from table */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0E6E54 */
int Obj_E6E54_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    uint8_t t23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    uint8_t s6 = rd8(m, obj + 0x06u);
    if (t23 == 0u && (uint8_t)(s6 - 1u) > 2u && (int8_t)(s6 - 1u) >= 0)
        return -1;

    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
    int16_t dx = (rd8(m, obj + 0x22u) == 0u) ? 0x016D : -0x000D;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u));

    wr8(m, obj + 0x23u, t23);
    if (t23 == 0u) {
        s6 = (uint8_t)(s6 - 1u);
        wr8(m, obj + 0x06u, s6);
        if ((int8_t)s6 < 0) {
            Obj_StartDying(m, obj);
            return 0;
        }
        uint32_t a1 = rd32(m, 0x000E6FC4u + (uint32_t)s6 * 4u);
        sub_0CFA86(m, obj, a1);
        wr8(m, obj + 0x23u, 4u);
    }

    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    for (int i = 0; i < 19; i++) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
        Obj_DrawSprite(m, obj);
    }
    return 0;
}

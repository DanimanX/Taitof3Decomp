#include "obj_1187DE.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* Branch item: frame from table, zoomed (+0x23) and offset by +0x30 while
 * +0x22 < 0x100 if this branch is `sel`, then its frame (lit / blinking) */
static void branch(const f3mem_t *m, uint32_t obj, uint32_t tbl, uint16_t y, int sel)
{
    uint16_t i = (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x3Eu));
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)((int32_t)(int16_t)i * 4)));
    wr16(m, obj + 0x14u, 0xB8u);
    wr16(m, obj + 0x18u, y);
    if (sel) {
        if ((int16_t)rd16(m, obj + 0x22u) >= 0x100)
            return;
        wr8(m, obj + 0x1Eu, rd8(m, obj + 0x23u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u)));
    }
    Obj_DrawSprite(m, obj);
    uint8_t f = rd8(m, F3_A5 + 0x32E1u);
    if (sel)
        wr32(m, obj + 0x10u, 0x0005A660u);
    else if (!(f & 0x02u) && (rd8(m, F3_A5 + 0x879u) & 0x02u))
        wr32(m, obj + 0x10u, 0x0005A6F0u);
    else
        wr32(m, obj + 0x10u, 0x0005A5D0u);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1187DE: route choice: current stage + two branches; the chosen one
 * (A5+0x32E1 bit 0) flies off once confirmed (bit 1) */
int Obj_1187DE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t i0 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x10u, rd32(m, 0x00118974u + i0 * 4u));
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x14u, 0x68u);
    wr16(m, obj + 0x18u, 0x54u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x10u, 0x0005A5D0u);
    Obj_DrawSprite(m, obj);
    branch(m, obj, 0x00118974u, 0x22u, (rd8(m, F3_A5 + 0x32E1u) & 0x01u) != 0u);
    branch(m, obj, 0x00118978u, 0x86u, (rd8(m, F3_A5 + 0x32E1u) & 0x01u) == 0u);
    if (rd8(m, F3_A5 + 0x32E1u) & 0x02u) {
        wr32(m, obj + 0x2Cu, rd32(m, obj + 0x2Cu) + 0x38000u);
        wr32(m, obj + 0x30u, rd32(m, obj + 0x30u) + 0x28000u);
        int16_t z = (int16_t)(rd16(m, obj + 0x22u) + 0x10u);
        wr16(m, obj + 0x22u, (uint16_t)(z > 0x100 ? 0x100 : z));
    }
    if (rd8(m, F3_A5 + 0x32E1u) & 0x80u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x001187DEu);
    return 0;
}

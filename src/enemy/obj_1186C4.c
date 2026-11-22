#include "obj_1186C4.h"
#include "../video/sprite.h"

static void draw2(const f3mem_t *m, uint32_t obj, uint32_t frame, uint16_t x, uint16_t y,
                  uint32_t frame2)
{
    wr32(m, obj + 0x10u, frame);
    wr16(m, obj + 0x14u, x);
    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x23u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu)));
    wr16(m, obj + 0x18u, y);
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    wr32(m, obj + 0x10u, frame2);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1186C4: three selected items (table 0x118974) with stretched trails */
int Obj_1186C4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t i0 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au);
    draw2(m, obj, rd32(m, 0x00118974u + i0 * 4u), 0x68u, 0x54u, 0x0005A5D0u);
    uint32_t i1 = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x3Eu));
    draw2(m, obj, rd32(m, 0x00118974u + i1 * 4u), 0xB8u, 0x22u, 0x0005A660u);
    i1 = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x3Eu));
    draw2(m, obj, rd32(m, 0x00118978u + i1 * 4u), 0xB8u, 0x86u, 0x0005A660u);
    wr32(m, obj + 0x2Cu, rd32(m, obj + 0x2Cu) - 0x38000u);
    wr32(m, obj + 0x30u, rd32(m, obj + 0x30u) - 0x28000u);
    int16_t z = (int16_t)(rd16(m, obj + 0x22u) - 0x10u);
    wr16(m, obj + 0x22u, (uint16_t)z);
    if (z >= 0) {
        wr32(m, obj + 0x00u, 0x001186C4u);
        return 0;
    }
    wr32(m, obj + 0x2Cu, 0u);
    wr32(m, obj + 0x30u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x00u, 0x001187DEu);
    return 0;
}

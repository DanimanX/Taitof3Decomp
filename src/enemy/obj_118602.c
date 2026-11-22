#include "obj_118602.h"
#include "../video/sprite.h"

static void draw_at(const f3mem_t *m, uint32_t obj, uint16_t x, uint16_t y, uint32_t frame)
{
    wr16(m, obj + 0x14u, (uint16_t)(x + rd16(m, obj + 0x2Cu)));
    wr16(m, obj + 0x18u, (uint16_t)(y + rd16(m, obj + 0x30u)));
    wr32(m, obj + 0x10u, frame);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x118602: three sprites zooming (+0x23) and sliding in */
int Obj_118602_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x23u));
    wr8(m, obj + 0x1Eu, rd8(m, obj + 0x23u));
    draw_at(m, obj, 0x68u, 0x54u, 0x0005A5D0u);
    draw_at(m, obj, 0xB8u, 0x22u, 0x0005A660u);
    draw_at(m, obj, 0xB8u, 0x86u, 0x0005A660u);
    wr32(m, obj + 0x2Cu, rd32(m, obj + 0x2Cu) - 0x38000u);
    wr32(m, obj + 0x30u, rd32(m, obj + 0x30u) - 0x28000u);
    int16_t z = (int16_t)(rd16(m, obj + 0x22u) - 0x10u);
    wr16(m, obj + 0x22u, (uint16_t)z);
    if (z >= 0) {
        wr32(m, obj + 0x00u, 0x00118602u);
        return 0;
    }
    wr16(m, obj + 0x22u, 0xF0u);
    wr32(m, obj + 0x2Cu, 0x00348000u);
    wr32(m, obj + 0x30u, 0x00258000u);
    wr32(m, obj + 0x00u, 0x001186C4u);
    return 0;
}

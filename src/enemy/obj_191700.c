#include "obj_191700.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x191700: slide left 5, wobble y by the top 7 bits of byte +0x1E */
int Obj_191700_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 5u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 2u));
    uint8_t b = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u);
    wr8(m, obj + 0x1Eu, b);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (b >> 1)));
    Obj_DrawSprite(m, obj);
    if (rd8(m, obj + 0x1Eu) != 0u) {
        wr32(m, obj, 0x191700u);
        return 0;
    }
    wr16(m, obj + 0x60u, 0x400u);
    wr32(m, obj, 0x191742u);
    return 0;
}

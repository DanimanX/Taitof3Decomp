#include "obj_BC680.h"
#include "../video/sprite.h"

/* ROM 0x0BC680: zoom in from the corner, then hand over to 0xBC70A */
int Obj_BC680_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = (uint16_t)(((uint16_t)(0u - rd16(m, obj + 0x60u)) + 0x000Fu) << 3);
    wr8(m, obj + 0x1Fu, (uint8_t)d0w);
    wr8(m, obj + 0x1Eu, (uint8_t)d0w);

    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x1Fu) * 0x98u;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (d0 << 8));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    d0 = (uint32_t)rd8(m, obj + 0x1Eu) * 0x60u;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (d0 << 8));
    Obj_DrawSprite(m, obj);

    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BC680u);
        return 0;
    }
    wr32(m, obj + 0x5Cu, 0x000BCB6Eu);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 0x00480000u);
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00200000u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x50u, 0u);
    wr32(m, obj + 0x58u, 0x000BCA0Eu);
    wr16(m, obj + 0x54u, 0u);
    wr16(m, obj + 0x60u, 0x00AFu);
    wr32(m, obj + 0x00u, 0x000BC70Au);
    return 0;
}

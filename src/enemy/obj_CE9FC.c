#include "obj_CE9FC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CE9FC: ring of 8 linked children (0xCEB3E) */
int Obj_CE9FC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du)));
    wr16(m, obj + 0x20u, rd8(m, obj + 0x0Eu));
    uint32_t d2 = 0u;
    for (int i = 0; i < 8; i++) {
        uint32_t c = Obj_Spawn(m, obj, 0xCEB3Eu, 5u, 2u, d2);
        if ((int32_t)c <= 0)
            break;
        wr32(m, c + 0x44u, 0u);
        wr32(m, c + 0x48u, 0u);
        d2 = c;
    }
    wr32(m, obj + 0x36u, d2);
    int16_t d0 = (int16_t)rd16(m, obj + 0x20u);
    int16_t c = (int16_t)rd16(m, 0x27E40u + 0x80u + (uint32_t)(int32_t)d0 * 2u);
    int16_t s = (int16_t)rd16(m, 0x27E40u + (uint32_t)(int32_t)d0 * 2u);
    wr32(m, obj + 0x2Cu, (uint32_t)((int32_t)c * 0xFA0) + rd32(m, obj + 0x14u));
    wr32(m, obj + 0x30u, (uint32_t)((int32_t)s * 0xFA0) + rd32(m, obj + 0x18u));
    wr16(m, obj + 0x42u, 0x1Cu);
    wr16(m, obj + 0x06u, 0x1Eu);
    wr32(m, obj, 0xCEAA4u);
    return 0;
}

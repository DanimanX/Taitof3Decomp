#include "obj_C8168.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0C8168: sparkle drifting along +0x20 with a wobble offset */
int Obj_C8168_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x22u) >> 8));
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    uint16_t d0 = rd16(m, obj + 0x3Au);
    wr8(m, obj + 0x3Bu, (uint8_t)(rd8(m, obj + 0x3Bu) + 4u));
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    int32_t p = (int32_t)s * (int32_t)(int16_t)rd16(m, obj + 0x3Eu);
    uint16_t d1w = (uint16_t)(((uint32_t)p << 2) >> 16);
    uint16_t d2w = d1w;
    d1w = (uint16_t)(0u - d1w);
    d0 = rd16(m, obj + 0x20u);
    uint32_t i = (uint32_t)(int32_t)(int16_t)d0 * 2u;
    int32_t d1 = (int32_t)(int16_t)d1w * (int32_t)(int16_t)rd16(m, 0x00027E40u + i);
    int32_t d2 = (int32_t)(int16_t)d2w * (int32_t)(int16_t)rd16(m, 0x00027EC0u + i);
    wr32(m, obj + 0x2Cu, (uint32_t)d1);
    wr32(m, obj + 0x30u, (uint32_t)d2);
    uint16_t sx = rd16(m, obj + 0x14u), sy = rd16(m, obj + 0x18u);
    wr16(m, obj + 0x14u, (uint16_t)(sx + rd16(m, obj + 0x2Cu)));
    wr16(m, obj + 0x18u, (uint16_t)(sy + rd16(m, obj + 0x30u)));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x18u, sy);
    wr16(m, obj + 0x14u, sx);
    int16_t t = (int16_t)(rd16(m, obj + 0x22u) - 0x20u);
    wr16(m, obj + 0x22u, (uint16_t)t);
    if (t > 0x400) {
        wr32(m, obj + 0x00u, 0x000C8168u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

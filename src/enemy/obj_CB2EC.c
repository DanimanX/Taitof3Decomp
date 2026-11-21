#include "obj_CB2EC.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0CB2EC: spiral around the screen centre while shrinking */
int Obj_CB2EC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0800u));
    int16_t r = (int16_t)rd16(m, obj + 0x22u);
    r = (int16_t)(r - (int16_t)(r >> 3));
    wr16(m, obj + 0x22u, (uint16_t)r);
    int32_t d0 = (int32_t)0x100 * (int32_t)r;
    d0 >>= 8;
    d0 >>= 6;
    wr16(m, obj + 0x42u, (uint16_t)((uint16_t)d0 + rd16(m, F3_A5 + 0x1D5Cu)));
    wr32(m, obj + 0x14u, 0x00A00000u);
    wr32(m, obj + 0x18u, 0x00740000u);
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));

    /* 0x0CB32C */
    uint16_t d1w = rd16(m, obj + 0x06u);
    if ((int16_t)d1w > 0x80) {
        wr32(m, obj + 0x10u, 0x00029940u);
    } else {
        wr32(m, obj + 0x10u, 0x0002997Cu);
        d1w = (uint16_t)(d1w + d1w);
    }
    d0 = (int32_t)(int16_t)-0x100 * (int32_t)(int16_t)d1w;
    d0 >>= 8;
    uint8_t zb = (uint8_t)((uint16_t)d0 + 0x0100u);
    wr8(m, obj + 0x1Fu, zb);
    wr8(m, obj + 0x1Eu, zb);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t p = (int32_t)(int16_t)w * (int32_t)(int16_t)rd16(m, a1);
    w = (uint16_t)((int16_t)(uint16_t)p >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + w));
    Obj_DrawSprite(m, obj);

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 8u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000CB2ECu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

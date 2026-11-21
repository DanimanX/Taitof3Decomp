#include "obj_CBC78.h"
#include "../video/sprite.h"

/* ROM 0x0CBC78: orbit the screen centre while +0x30 counts down */
int Obj_CBC78_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t c30 = (int16_t)(rd16(m, obj + 0x30u) - 2u);
    wr16(m, obj + 0x30u, (uint16_t)c30);
    if (c30 <= 0 || (rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return 0;
    }

    /* 0x0CBBC4 */
    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));
    uint8_t s32 = rd8(m, obj + 0x32u);
    if (s32 != 0u) {
        uint32_t tbl = rd32(m, obj + 0x0Cu);
        uint16_t idx = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        uint32_t sh = s32 & 63u;
        idx = (sh >= 16u) ? 0u : (uint16_t)(idx >> sh);
        wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)idx * 4u));
    }

    /* 0x0CBBF2 */
    uint16_t c2c = rd16(m, obj + 0x2Cu);
    c2c = (uint16_t)(c2c - (uint16_t)(c2c >> 5));
    wr16(m, obj + 0x2Cu, c2c);
    int32_t d1 = (int32_t)(0x0100u * (uint32_t)c2c);
    d1 >>= 8;
    d1 >>= 6;
    uint16_t r = (uint16_t)((uint16_t)d1 + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    int32_t x = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u) * (int32_t)(int16_t)r;
    wr32(m, obj + 0x14u, (uint32_t)x + 0x00A00000u);
    int32_t y = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u) * (int32_t)(int16_t)r;
    wr32(m, obj + 0x18u, (uint32_t)y + 0x00740000u);

    int32_t z = (int32_t)(int16_t)(-0x0100) * (int32_t)(int16_t)rd16(m, obj + 0x30u);
    z >>= 8;
    uint8_t zb = (uint8_t)((uint16_t)z + 0x0100u);
    wr8(m, obj + 0x1Fu, zb);
    wr8(m, obj + 0x1Eu, zb);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t d0 = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (int16_t)((int16_t)(uint16_t)prod >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CBC78u);
    return 0;
}

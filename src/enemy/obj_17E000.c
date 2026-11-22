#include "obj_17E000.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17BDA2: place the object at parent + polar offset from arg (+0xC) */
static void sub_17BDA2(const f3mem_t *m, uint32_t obj)
{
    int16_t d0 = (int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint16_t d2 = rd8(m, obj + 0x0Eu);
    uint16_t d3 = rd8(m, obj + 0x0Fu);
    wr16(m, obj + 0x16u, (uint16_t)d0);
    wr16(m, obj + 0x1Au, (uint16_t)d1);
    wr16(m, obj + 0x20u, 0u);
    wr8(m, obj + 0x20u, (uint8_t)d3);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + d3 * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + d3 * 2u);
    d2 = (uint16_t)(d2 << 2);
    uint32_t p0 = (uint32_t)((int32_t)c * (int32_t)(int16_t)d2);
    uint32_t p1 = (uint32_t)((int32_t)s * (int32_t)(int16_t)d2);
    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)(p0 >> 16)));
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)(p1 >> 16)));
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
}

/* cmp2.w (0x1849F8) / (0x1849FC): on-screen test of +0x50 / +0x54 */
static int in_bounds(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu))
        return 0;
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu))
        return 0;
    return 1;
}

/* ROM 0x17E000: homing enemy spawn: target = player (A5+0x1D28) */
int Obj_17E000_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x4Cu, tgt);
    if (tgt == 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    sub_17BDA2(m, obj);
    if (!in_bounds(m, obj)) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_Spawn(m, obj, 0x0017C856u, 0x0Bu, 1u, 0u);
    wr16(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x0017E05Au);
    return 0;
}

#include "obj_CAF90.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CB14A: place at the screen centre scaled by +0x1E, draw */
static void sub_0CB14A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0x0074u);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t p = (int32_t)(int16_t)d0w * (int32_t)(int16_t)rd16(m, a1);
    d0w = (uint16_t)((int16_t)(uint16_t)p >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
    d0w = (uint16_t)(d0w << 2);
    d0w = (uint16_t)(0u - d0w);
    wr16(m, F3_A5 + 0x1D5Cu, d0w);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x0CB180: every other frame, spawn 4 x 0xCB2C6 at 90-degree steps */
static void sub_0CB180(const f3mem_t *m, uint32_t obj, uint32_t d1_hi)
{
    uint16_t t = rd16(m, F3_A5 + 0x878u);
    if ((t & 0x02u) != 0u)
        return;
    uint16_t d1w = (uint16_t)(t << 9);
    for (int i = 0; i < 4; i++) {
        Obj_Spawn(m, obj, 0x000CB2C6u, 1u, 2u, d1_hi | d1w);
        d1w = (uint16_t)(d1w + 0x4000u);
    }
}

/* ROM 0x0CAF90 */
int Obj_CAF90_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0x000CB0F8u);
    sub_0CB14A(m, obj);
    /* D1 high word after Obj_DrawSprite (0x1282): bfins of (width - 1) at
     * 0x1308; the single-sprite path (def == 0x00010001) leaves the caller's
     * D1, which is not known here. */
    uint32_t def = rd32(m, obj + 0x10u);
    if ((rd16(m, F3_A5 + 0x878u) & 0x02u) == 0u && rd32(m, def) == 0x00010001u)
        return -1;
    uint32_t d1_hi = (uint32_t)(uint16_t)(rd16(m, def) - 1u) << 16;
    sub_0CB180(m, obj, d1_hi);
    uint16_t c = rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(c - 1u));
    if (c > 1u) {                          /* bhi */
        wr32(m, obj + 0x00u, 0x000CAF90u);
        return 0;
    }
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr32(m, obj + 0x00u, 0x000CAFB8u);
    return 0;
}

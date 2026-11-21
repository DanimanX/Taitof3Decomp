#include "obj_CB422.h"
#include "../objects/obj_pool.h"

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} (max 0x20) */
static void sub_001660(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, a2, count);
    wr32(m, a2 + 2u, src);
    wr16(m, a2 + 6u, dst);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x0CB422: palette flash (alternates each tick) */
int Obj_CB422_Tick(const f3mem_t *m, uint32_t obj)
{
    /* movea.l (0xde,PC,D0w*4),A1 at 0xCB426 -> table 0xCB506 */
    uint16_t sel = rd16(m, F3_A5 + 0x32DCu);
    uint32_t a1 = rd32(m, 0x000CB506u + (uint32_t)((int32_t)(int16_t)sel * 4));

    while ((int16_t)rd16(m, a1) >= 0) {
        uint32_t a2 = rd32(m, a1); a1 += 4u;
        uint16_t d0 = rd16(m, a1); a1 += 2u;
        uint16_t d1 = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
        uint16_t d2 = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1 += 1u;
        if (rd8(m, obj + 0x22u) == 0u)
            a2 = 0x000CB486u;              /* lea (0x3c,PC) at 0xCB448 */
        d1 = (uint16_t)(d1 << 2);
        d0 = (uint16_t)(d0 + d1);
        a2 += (uint32_t)(int32_t)(int16_t)d1;
        sub_001660(m, a2, d0, d2);
    }

    /* 0x0CB466 */
    uint8_t f = (uint8_t)~rd8(m, obj + 0x22u);
    wr8(m, obj + 0x22u, f);
    if (f != 0u || (rd8(m, F3_A5 + 0x1D58u) & 0x02u)) {
        wr32(m, obj + 0x00u, 0x000CB422u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

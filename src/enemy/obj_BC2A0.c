#include "obj_BC2A0.h"
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

/* ROM 0x0BC2F0: rotate the colour longs at A1 by one and queue the upload */
static void sub_0BC2F0(const f3mem_t *m, uint32_t a1, uint16_t d1, uint16_t d2)
{
    uint32_t d3 = rd32(m, a1);
    int16_t d7 = 0;
    int16_t d6 = (int16_t)(uint16_t)(d2 - 1u);
    do {
        wr32(m, a1 + (uint32_t)d7 * 4u, rd32(m, a1 + 4u + (uint32_t)d7 * 4u));
        d7++;
    } while (d7 < d6);
    wr32(m, a1 + (uint32_t)d7 * 4u, d3);
    sub_001660(m, a1, d1, d2);
}

/* ROM 0x0BC2A0: palette cycling */
int Obj_BC2A0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd16(m, obj + 0x60u) & 0x3u) == 0u)
        sub_0BC2F0(m, 0x0040B3AEu, 0x0E58u, 0x000Au);
    if ((rd16(m, obj + 0x60u) & 0x7u) == 0u)
        sub_0BC2F0(m, 0x0040B42Eu, 0x0F70u, 0x0004u);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BC2A0u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

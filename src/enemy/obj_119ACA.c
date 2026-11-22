#include "obj_119ACA.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

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

/* ROM 0x119ACA */
int Obj_119ACA_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_DrawSprite(m, obj);
    uint16_t x = (uint16_t)(rd16(m, obj + 0x14u) - 0x14u);
    wr16(m, obj + 0x14u, x);
    if ((int16_t)x > -0xA0) {
        wr32(m, obj + 0x00u, 0x00119ACAu);   /* 0x119AC2 */
        return 0;
    }
    sub_001660(m, 0x00017200u, 0x4580u, 0x20u);
    sub_001660(m, 0x00016900u, 0x4080u, 0x20u);
    sub_001660(m, 0x00016980u, 0x4100u, 0x20u);
    Obj_Free(m, obj);
    return 0;
}

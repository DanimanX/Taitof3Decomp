#include "obj_111450.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x1115F2: scroll-derived draw angle word (shared by the 0x111450 family) */
static uint16_t angle_1115F2(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(((uint16_t)(0u - rd16(m, F3_A5 + 0x4218u)) + 0x7Fu) & 0xFFu);
    uint16_t d1, q, hi;
    if ((int16_t)d0 <= 0x73) {
        d1 = (uint16_t)(0x74u - d0);
        q = (uint16_t)(((uint32_t)d1 << 6) / 0x28u);
        hi = (uint16_t)((((uint32_t)q * 0xE000u) << 1) >> 16);
        d0 = (uint16_t)(d0 - hi);
    } else {
        d1 = (uint16_t)(d0 - 0x73u);
        q = (uint16_t)(((uint32_t)d1 << 6) / 0x28u);
        hi = (uint16_t)((((uint32_t)q * 0xE000u) << 1) >> 16);
        d0 = (uint16_t)(d0 + hi);
    }
    return (uint16_t)(d0 - 0x30u);
}

/* ROM 0x111450: step the anim table at 0x11155C, drift +0x14/+0x18, free when it ends */
int Obj_111450_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t tbl = 0x11155Cu;
    Obj_AnimFrameStep(m, obj, tbl);
    int16_t off = (int16_t)rd16(m, obj + 0x3Au);
    if ((int16_t)rd16(m, tbl + (uint32_t)(int32_t)off) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x18000u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x8000u);
    wr16(m, obj + 0x18u, angle_1115F2(m));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x111450u);
    return 0;
}

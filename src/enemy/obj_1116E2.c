#include "obj_1116E2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x1116E2: background object tied to the scroll position */
int Obj_1116E2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = ((0u - rd32(m, F3_A5 + 0x4218u)) + 0x007FFFFFu) & 0x00FFFFFFu;
    uint32_t d1;
    if ((int32_t)d0 > 0x730000) {
        d1 = (d0 - 0x730000u) >> 5;
        d1 = (d1 / 5u) & 0xFFFFu;          /* divu.w #5 (quotient fits) */
        d0 += d1 * 0x1C0u;
    } else {
        d1 = (0x740000u - d0) >> 5;
        d1 = (d1 / 5u) & 0xFFFFu;
        d0 -= d1 * 0x1C0u;
    }
    d0 -= 0x80000u;
    wr32(m, obj + 0x18u, d0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 2u * rd32(m, F3_A5 + 0x4228u));
    wr32(m, obj + 0x14u, 0u);
    Obj_Animate(m, obj, 0x00111776u);
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, F3_A5 + 0x4204u) > 0x1200) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x001116E2u);
    return 0;
}

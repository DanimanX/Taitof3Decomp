#include "obj_BBBF4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBBBF4 */
int Obj_BBBF4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBBF4: odd frames skip straight to the age-out */
    if ((rd16(m, obj + 0x60u) & 0x0001u) == 0u) {
        int16_t d0w = (int16_t)rd16(m, obj + 0x60u);
        int16_t d1w = d0w;
        /* muls.w #0x3800 / #0x2400 then <<3, +0x140000 / +0x100000 */
        int32_t dx = (int32_t)d0w * 0x3800;
        int32_t dy = (int32_t)d1w * 0x2400;
        dx = (int32_t)((uint32_t)dx << 3);
        dy = (int32_t)((uint32_t)dy << 3);
        dx += 0x140000;
        dy += 0x100000;
        wr32(m, obj + 0x44u, (uint32_t)dx);
        wr32(m, obj + 0x48u, (uint32_t)dy);
        wr32(m, obj + 0x14u, (uint32_t)dx);
        wr32(m, obj + 0x18u, (uint32_t)dy);
        wr32(m, obj + 0x10u, 0x00055220u);
        Obj_DrawSprite(m, obj);

        /* 0x0BBC40: a short vertical column of flames up to 0x1200000 */
        uint32_t d0 = rd32(m, obj + 0x44u);
        do {
            d0 += 0x100000u;
            if (d0 >= 0x01200000u)
                d0 = 0x01200000u;
            wr32(m, obj + 0x14u, d0);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
            wr32(m, obj + 0x10u, 0x00055228u);
            Obj_DrawSprite(m, obj);
        } while (d0 != 0x01200000u);

        /* 0x0BBC82 */
        d0 = rd32(m, obj + 0x48u);
        do {
            d0 += 0x100000u;
            if (d0 >= 0x00A00000u)
                d0 = 0x00A00000u;
            wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
            wr32(m, obj + 0x18u, d0);
            wr32(m, obj + 0x10u, 0x00055238u);
            Obj_DrawSprite(m, obj);
        } while (d0 != 0x00A00000u);
    }

    /* 0x0BBCC4 */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BBBEC */
        wr32(m, obj, 0x000BBBF4u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

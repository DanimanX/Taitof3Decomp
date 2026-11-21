#include "obj_BBD00.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24: frame animator over a {dur.w, sprdef.l} table A1; frame
 * index obj+0x3A (byte offset), timer obj+0x3E */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0w = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int16_t)d0w) <= 0)   /* tst.w */
            return;
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
        if ((int16_t)rd16(m, obj + 0x3Eu) <= (int16_t)rd16(m, a1 + (uint32_t)(int16_t)d0w)) {
            /* 0x0CBD4A */
            wr32(m, obj + 0x10u, rd32(m, a1 + 0x02u + (uint32_t)(int16_t)d0w));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0w + 6u));
    }
}

/* ROM 0xBBD00 */
int Obj_BBD00_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBD00: animate from table (A0)+0x44, draw, then age out */
    sub_0CBD24(m, obj, rd32(m, obj + 0x44u));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BBCF8 */
        wr32(m, obj, 0x000BBD00u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

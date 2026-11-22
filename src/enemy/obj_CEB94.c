#include "obj_CEB94.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBDAE: append to the player-hit list (A5+0x906) */
static void sub_0CBDAE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}


/* ROM 0x0CEB94: chain link, drawn 8px up-left of its position */
int Obj_CEB94_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u) {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t <= 0) {
            uint32_t d0 = rd32(m, obj + 0x48u);
            if ((int32_t)d0 > 0)
                wr32(m, d0 + 0x44u, 0u);
            else
                wr32(m, rd32(m, obj + 0x08u) + 0x36u, 0u);
            Obj_Free(m, obj);
            return 0;
        }
        sub_0CBDAE(m, obj);
        uint8_t b = (uint8_t)(rd8(m, obj + 0x21u) + 8u);
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, 0xCC10Eu + ((b >> 6) & 3u)) ^ 1u));
        wr32(m, obj + 0x10u, rd32(m, 0xCEC20u + ((b >> 4) & 7u) * 4u));
        uint16_t x = rd16(m, obj + 0x14u), y = rd16(m, obj + 0x18u);
        wr16(m, obj + 0x14u, (uint16_t)(x - 8u));
        wr16(m, obj + 0x18u, (uint16_t)(y - 8u));
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x18u, y);
        wr16(m, obj + 0x14u, x);
    }
    wr32(m, obj, 0xCEB94u);
    return 0;
}

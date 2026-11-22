#include "obj_119090.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x119090: draw 8 sprites (table 0x1190FC: {dx.w, frame.l}), fading in
 * with a staggered zoom from +0x22; +0x22 -= 6 per frame */
int Obj_119090_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d1 = rd16(m, obj + 0x22u);
    uint32_t a1 = 0x001190FCu;
    wr16(m, obj + 0x14u, 0x40u);
    wr16(m, obj + 0x18u, 0x4Cu);
    for (int i = 0; i < 8; i++) {
        uint16_t dx = rd16(m, a1); a1 += 2u;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
        if ((int16_t)d1 < 0x100) {
            uint16_t d2 = (int16_t)d1 < 0 ? 0u : d1;
            wr8(m, obj + 0x1Eu, (uint8_t)d2);
            Obj_DrawSprite(m, obj);
        }
        d1 = (uint16_t)(d1 + 0x40u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 2u));
    }
    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) - 6u));
    if (rd8(m, F3_A5 - 0x144Bu) & 0x08u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00119090u);
    return 0;
}

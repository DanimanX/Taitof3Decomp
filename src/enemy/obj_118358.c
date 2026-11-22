#include "obj_118358.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x118358: triangular grid of 28 sprites (frames 0x1183FE), sliding up
 * by +0x30; the cursor cell (A5+0x32DC) blinks */
int Obj_118358_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x001183FEu;
    uint16_t d3 = 0u;
    wr16(m, obj + 0x14u, (uint16_t)(0x14u + rd16(m, obj + 0x2Cu)));
    for (uint16_t d0 = 0u; d0 < 7u; d0++) {
        uint16_t d4 = (uint16_t)(0xA0u - 6u * d0);
        wr16(m, obj + 0x18u, (uint16_t)(d4 + rd16(m, obj + 0x30u)));
        for (uint16_t i = 0u; i <= d0; i++) {
            wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
            int hide = (rd8(m, F3_A5 + 0x32E1u) & 0x02u)
                && d3 == rd16(m, F3_A5 + 0x32DCu)
                && (rd8(m, F3_A5 + 0x879u) & 0x04u);
            if (!hide)
                Obj_DrawSprite(m, obj);
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0Cu));
            d3++;
        }
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
    }
    int16_t y = (int16_t)(rd16(m, obj + 0x30u) - 8u);
    wr16(m, obj + 0x30u, (uint16_t)(y < 0 ? 0 : y));
    if (rd8(m, F3_A5 + 0x32E1u) & 0x80u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00118358u);
    return 0;
}

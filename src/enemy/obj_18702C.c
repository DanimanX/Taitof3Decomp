#include "obj_18702C.h"
#include "../video/sprite.h"

/* ROM 0x1870BA / 0x18712A / 0x18719A: blit up to 8 16x16 compressed tilemap
 * blocks across a horizontal playfield strip based on the mask word at *a3. */
static void blit_strip(const f3mem_t *m, uint32_t obj, uint32_t a2, uint32_t *a3,
                       uint32_t scroll_x_addr, uint32_t scroll_y_addr,
                       uint32_t tilemap_base)
{
    uint16_t d0 = (uint16_t)(rd16(m, scroll_x_addr) + rd16(m, obj + 0x50u));
    uint16_t d1 = (uint16_t)(rd16(m, scroll_y_addr) + rd16(m, a2 + 8u));
    d0 = (d0 & 0x01FFu) >> 4;
    d1 = (d1 & 0x01FFu) >> 4;

    uint32_t a4 = rd32(m, a2 + 4u);
    uint16_t mask = rd16(m, *a3);
    *a3 += 2u;

    for (int d2 = 7; d2 >= 0; d2--) {
        if ((mask & (1u << d2)) == 0u) {
            uint32_t dst = tilemap_base + (uint32_t)d1 * 0x80u + (uint32_t)d0 * 4u;
            Gfx_DrawCompressedTilemap(m, a4, dst, 0u);
        }
        d0 = (uint16_t)((d0 + 1u) & 0x1Fu);
    }
}

/* ROM 0x18702C: background tilemap strip updater triggered by A5+0x4B3B bit 2. */
int Obj_18702C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x50u, rd16(m, F3_A5 + 0x4B6Eu));
    wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x4B70u));

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x04u) != 0u &&
        (int16_t)rd16(m, F3_A5 + 0x4B6Eu) > -30) {
        uint32_t a3 = 0x0018720Au;
        int16_t d0 = (int16_t)rd16(m, obj + 0x54u);
        for (int i = 0; i < 5; i++) {
            if (d0 < (int16_t)rd16(m, a3))
                break;
            a3 += 0x0Cu;
        }
        a3 += 2u;

        uint32_t a2 = 0x00187088u;
        for (int i = 0; i < 5; i++) {
            uint32_t fn = rd32(m, a2);
            if (fn == 0x001870BAu)
                blit_strip(m, obj, a2, &a3, 0x00402258u, 0x0040225Cu, 0x00610000u);
            else if (fn == 0x0018712Au)
                blit_strip(m, obj, a2, &a3, 0x00402260u, 0x00402264u, 0x00611000u);
            else if (fn == 0x0018719Au)
                blit_strip(m, obj, a2, &a3, 0x00402268u, 0x0040226Cu, 0x00612000u);
            a2 += 0x0Au;
        }
    }

    wr32(m, obj + 0x00u, 0x0018702Cu);
    return 0;
}

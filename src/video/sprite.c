/* Darius Gaiden (Taito F3) — Obj_DrawSprite, decompiled from ROM
 * 0x001282..0x00140E. Register-level faithful: D4 carries the F3 "block"
 * control bits (12: in block, 13/14/15, 31: row has an anchor sprite). */
#include "sprite.h"

static void draw_multi_grid(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t a2,
                            uint16_t d0w, uint16_t d1w, int16_t d7)
{
    uint8_t colour = rd8(m, obj + 0x1D);
    uint16_t zoom = rd16(m, obj + 0x1E);
    int16_t d6;
    uint32_t d4 = rd16(m, obj + 0x1C) & 0xFF00u;
    switch (rd8(m, obj + 0x1C)) {                     /* jump table 0x12B8 */
    case 0:
        d6 = 4; d7 = (int16_t)(d7 << 2);
        break;
    case 1:
        d6 = 4; d7 = (int16_t)-(int16_t)(d7 << 2);
        a2 -= (uint32_t)(int32_t)(int16_t)((int16_t)d1w * d7);
        break;
    case 2:
        d6 = -4; d7 = (int16_t)(d7 << 2);
        a2 += (uint32_t)((int32_t)(int16_t)d0w * 4);
        break;
    default: /* 3 (larger values jump through ROM data; not seen) */
        d6 = -4; d7 = (int16_t)-(int16_t)(d7 << 2);
        a2 -= (uint32_t)(int32_t)(int16_t)((int16_t)d1w * d7);
        a2 += (uint32_t)((int32_t)(int16_t)d0w * 4);
        break;
    }
    uint16_t cols = d0w, rows = d1w;
    uint16_t d5w = (uint16_t)(rd8(m, obj + 0x1E) - 0x100);
    uint16_t x = rd16(m, obj + 0x14);
    wr16(m, a1 + 4, x & 0x0FFFu);
    do {
        uint32_t a3 = a2;
        uint16_t d2w = (uint16_t)(rd16(m, obj + 0x18) << 4);
        d4 &= ~((1u << 12) | (1u << 13) | (1u << 31));
        uint16_t c = cols;
        do {
            uint16_t tile = rd16(m, a2);
            wr16(m, a1, tile);
            if (tile) {
                uint16_t attr = (uint16_t)(rd16(m, a2 + 2) ^ (uint16_t)d4);
                if (colour) attr = (uint16_t)((attr & 0xFF00u) | colour);
                wr16(m, a1 + 8, attr);
                wr16(m, a1 + 2, zoom);
                int was = (d4 >> 12) & 1;
                d4 |= 1u << 12;
                if (!was) {
                    d4 |= (1u << 13) | (1u << 14) | (1u << 31);
                    d4 &= ~(1u << 15);
                    wr16(m, a1 + 6, (uint16_t)((d2w >> 4) & 0x0FFFu));
                }
                a1 += 0x10;
            } else {
                d4 &= ~((1u << 12) | (1u << 13));
            }
            a2 += (uint32_t)(int32_t)d6;
            d2w = (uint16_t)(d2w - d5w);
        } while (c-- != 0);
        a2 = a3 + (uint32_t)(int32_t)d7;
        if (!(d4 & (1u << 31))) {
            wr16(m, a1 + 8, (uint16_t)d4);
            wr16(m, a1 + 2, zoom);
            d4 |= 1u << 14;
            a1 += 0x10;
        }
        d4 |= 1u << 15;
    } while (rows-- != 0);
    wr32(m, SPR_CURSOR, a1);
}

void Obj_DrawSprite(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, SPR_CURSOR);
    uint32_t a2 = rd32(m, obj + 0x10);
    uint8_t colour = rd8(m, obj + 0x1D);
    uint16_t zoom = rd16(m, obj + 0x1E);

    if (rd32(m, a2) == 0x00010001u) {                 /* single sprite, 0x13CE */
        wr16(m, a1 + 2, zoom);
        uint32_t d0 = ((uint32_t)rd16(m, obj + 0x14) << 16) | rd16(m, obj + 0x18);
        wr32(m, a1 + 4, d0 & 0x0FFF0FFFu);
        wr16(m, a1, rd16(m, a2 + 4));
        uint16_t attr = (uint16_t)(rd16(m, a2 + 6) ^ (rd16(m, obj + 0x1C) & 0xFF00u));
        if (colour) attr = (uint16_t)((attr & 0xFF00u) | colour);
        wr16(m, a1 + 8, attr);
        wr32(m, SPR_CURSOR, a1 + 0x10);
        return;
    }

    uint16_t d0w = rd16(m, a2), d1w = rd16(m, a2 + 2);
    a2 += 4;
    int16_t d7 = (int16_t)d0w;
    d0w--; d1w--;
    draw_multi_grid(m, obj, a1, a2, d0w, d1w, d7);
}

/* ROM 0x001410 */
void Obj_DrawSpriteSub(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1, uint16_t d2)
{
    uint32_t a1 = rd32(m, SPR_CURSOR);
    uint32_t a2 = rd32(m, obj + 0x10);
    int16_t d7 = (int16_t)rd16(m, a2);
    int16_t d6 = (int16_t)rd16(m, a2 + 2);
    a2 += 4;
    if (d2 & 1u) {
        int16_t d3 = (int16_t)(d7 - (int16_t)d1);
        a2 += (uint32_t)((int32_t)d3 * 4);
    }
    if (d2 & 2u) {
        int16_t d3 = (int16_t)(d6 - (int16_t)d0);
        a2 += (uint32_t)((int32_t)d3 * (int32_t)d7 * 4);
    }
    uint16_t cols = (uint16_t)(d1 - 1u);
    uint16_t rows = (uint16_t)(d0 - 1u);
    draw_multi_grid(m, obj, a1, a2, cols, rows, d7);
}

/* ROM 0x001232 */
void Video_SetSpriteScroll(const f3mem_t *m)
{
    uint32_t a0 = 0x00600020u;
    uint16_t x = rd16(m, F3_A5 - 0x0CACu) & 0x0FFFu;
    uint16_t y = rd16(m, F3_A5 - 0x0CA8u) & 0x0FFFu;
    wr16(m, a0 + 0x0004u, (uint16_t)((rd16(m, a0 + 0x0004u) & 0xF000u) | x));
    wr16(m, a0 + 0x8004u, (uint16_t)((rd16(m, a0 + 0x8004u) & 0xF000u) | x));
    wr16(m, a0 + 0x0006u, (uint16_t)((rd16(m, a0 + 0x0006u) & 0xF000u) | y));
    wr16(m, a0 + 0x8006u, (uint16_t)((rd16(m, a0 + 0x8006u) & 0xF000u) | y));
}

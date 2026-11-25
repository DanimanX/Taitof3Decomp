/* Darius Gaiden (Taito F3) — Obj_DrawSprite, decompiled from ROM
 * 0x001282..0x00140E. Register-level faithful: D4 carries the F3 "block"
 * control bits (12: in block, 13/14/15, 31: row has an anchor sprite). */
#include "sprite.h"
#include "../enemy/obj_1194C2.h"

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

/* ROM 0x005BE8 */
static uint32_t fill_lineram_run(const f3mem_t *m, uint32_t a1, uint16_t val)
{
    for (int i = 0; i < 0xE8; i++) {
        wr16(m, a1, val);
        a1 += 2u;
    }
    return a1 + 0x30u;
}

/* ROM 0x0055C8 */
void Gfx_DrawTilemapBlock(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t flags)
{
    uint32_t a0 = src;
    uint32_t a1 = dst;
    int16_t rows = (int16_t)rd16(m, a0);
    a0 += 2u;
    int16_t cols = (int16_t)rd16(m, a0);
    a0 += 2u;
    uint16_t d5 = (uint16_t)(flags << 8);
    uint16_t d4 = (uint16_t)(flags << 1);
    d4 = (uint16_t)((d4 & 0xFF00u) | (uint8_t)(d4 << 6));
    d4 >>= 5;
    int16_t step_col = 2, step_row = 0x80;
    uint32_t mode = rd32(m, 0x00005616u + d4);
    if (mode == 0x00005630u) {
        step_col = -2;
        a1 += (int16_t)(cols - 2);
    } else if (mode == 0x0000563Eu) {
        step_row = -0x80;
        a1 += (int16_t)((rows - 1) << 7);
    } else if (mode == 0x00005650u) {
        step_col = -2;
        step_row = -0x80;
        a1 += (int16_t)(cols - 2);
        a1 += (int16_t)((rows - 1) << 7);
    }
    do {
        uint32_t a2 = a1;
        int16_t c = cols;
        do {
            wr16(m, a1, (uint16_t)(rd16(m, a0) ^ d5));
            a0 += 2u;
            a1 += step_col;
            c--;
        } while (c > 0);
        a1 = a2 + step_row;
        rows--;
    } while (rows > 0);
}

/* ROM 0x005690 */
uint32_t Gfx_DrawHex(const f3mem_t *m, uint32_t val, uint16_t digits, uint32_t dst, uint16_t attr)
{
    uint16_t d3 = (uint16_t)((attr * 2u) & 0x3Eu);
    uint32_t a0 = dst + (int16_t)(digits << 1);
    uint32_t ret = a0;
    int16_t d1 = (int16_t)digits - 1;
    while (d1 >= 0) {
        uint16_t nib = (uint16_t)(val & 0xFu);
        uint8_t ch = (uint8_t)(nib < 10u ? nib + 0x30u : nib + 0x37u);
        a0 -= 1u;
        wr8(m, a0, ch);
        a0 -= 1u;
        wr8(m, a0, (uint8_t)d3);
        val >>= 4;
        d1--;
    }
    return ret;
}

/* ROM 0x00571A */
uint32_t Gfx_DrawDecimal(const f3mem_t *m, uint32_t val, uint16_t digits, uint32_t dst, uint16_t attr)
{
    uint16_t d2 = (uint16_t)(((attr & 0x3Fu) << 1) << 8);
    uint32_t a0 = dst + (int16_t)(digits << 1);
    int16_t d1 = (int16_t)(digits - 1u);
    uint32_t end = a0;
    uint32_t d4 = val & 0xFFFFu;
    uint32_t d3 = (val >> 16) & 0xFFFFu;
    do {
        if ((d3 & 0xFFFFu) == 0u && (d4 & 0xFFFFu) == 0u) {
            if (a0 == end) {
                d2 = (uint16_t)((d2 & 0xFF00u) | (uint8_t)((d2 & 0xFFu) + 0x30u));
            } else {
                d2 = (uint16_t)((d2 & 0xFF00u) | 0x30u);
            }
        } else {
            uint32_t q3 = d3 / 10u;
            uint32_t r3 = d3 % 10u;
            d4 += r3 << 16;
            d3 = q3;
            uint32_t q4 = d4 / 10u;
            uint32_t r4 = d4 % 10u;
            d4 = q4;
            d2 = (uint16_t)((d2 & 0xFF00u) | (uint8_t)((r4 & 0xFFu) + 0x30u));
        }
        a0 -= 2u;
        wr16(m, a0, d2);
        d1--;
    } while (d1 >= 0);
    return end;
}

/* ROM 0x005880 */
void Gfx_ClearTextLayer(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x800u; i++) {
        wr32(m, 0x0061C000u + i * 4u, 0u);
    }
    wr32(m, F3_A5 - 0x5D88u, 0u);
    wr16(m, F3_A5 - 0x5D84u, 0u);
    wr32(m, F3_A5 - 0x5D60u, 0u);
    wr16(m, F3_A5 - 0x5D5Cu, 0u);
}

/* ROM 0x0058AE */
void Gfx_ClearPf3(const f3mem_t *m)
{
    uint32_t a0 = 0x0061C000u;
    for (int r = 0; r < 30; r++) {
        for (int c = 0; c < 20; c++) {
            wr32(m, a0 + (uint32_t)c * 4u, 0u);
        }
        a0 += 0x80u;
    }
}

/* ROM 0x0058DA */
void Gfx_ClearPf0(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x400u; i++) {
        wr32(m, 0x00610000u + i * 4u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x0062A000u + i * 2u, 0u);
    }
    wr32(m, F3_A5 - 0x5DA8u, 0u);
    wr16(m, F3_A5 - 0x5DA4u, 0u);
    wr32(m, F3_A5 - 0x5D80u, 0u);
    wr16(m, F3_A5 - 0x5D7Cu, 0u);
}

/* ROM 0x005918 */
void Gfx_ClearPf0b(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x400u; i++) {
        wr32(m, 0x00611000u + i * 4u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x0062A200u + i * 2u, 0u);
    }
    wr32(m, F3_A5 - 0x5DA0u, 0u);
    wr16(m, F3_A5 - 0x5D9Cu, 0u);
    wr32(m, F3_A5 - 0x5D78u, 0u);
    wr16(m, F3_A5 - 0x5D74u, 0u);
}

/* ROM 0x005956 */
void Gfx_ClearPf1AndLineRam(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x400u; i++) {
        wr32(m, 0x00612000u + i * 4u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x0062A400u + i * 2u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x00628400u + i * 2u, 0x0080u);
        wr16(m, 0x00624400u + i * 2u, (uint16_t)(rd16(m, 0x00624400u + i * 2u) & 0xFE00u));
    }
    wr32(m, F3_A5 - 0x5D98u, 0u);
    wr16(m, F3_A5 - 0x5D94u, 0u);
    wr32(m, F3_A5 - 0x5D70u, 0u);
    wr16(m, F3_A5 - 0x5D6Cu, 0u);
}

/* ROM 0x0059B4 */
void Gfx_ClearPf2AndLineRam(const f3mem_t *m)
{
    for (uint32_t i = 0; i < 0x400u; i++) {
        wr32(m, 0x00613000u + i * 4u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x0062A600u + i * 2u, 0u);
    }
    for (uint32_t i = 0; i < 0x100u; i++) {
        wr16(m, 0x00628600u + i * 2u, 0x0080u);
        wr16(m, 0x00624600u + i * 2u, (uint16_t)(rd16(m, 0x00624600u + i * 2u) & 0xFE00u));
    }
    wr32(m, F3_A5 - 0x5D90u, 0u);
    wr16(m, F3_A5 - 0x5D8Cu, 0u);
    wr32(m, F3_A5 - 0x5D68u, 0u);
    wr16(m, F3_A5 - 0x5D64u, 0u);
}

/* ROM 0x005A12 */
void Gfx_ClearAllLayers(const f3mem_t *m)
{
    Gfx_ClearTextLayer(m);
    Gfx_ClearPf0(m);
    Gfx_ClearPf0b(m);
    Gfx_ClearPf1AndLineRam(m);
    Gfx_ClearPf2AndLineRam(m);
}

/* ROM 0x005A86 */
void Gfx_UploadTextTiles(const f3mem_t *m, uint32_t src, uint16_t slot)
{
    uint32_t a1 = 0x0061E000u + (uint16_t)(slot << 5);
    uint32_t a2 = 0x0001120Au;
    int16_t count = (int16_t)rd16(m, src);
    src += 2u;
    do {
        uint16_t idx = rd16(m, src);
        src += 2u;
        uint32_t a3 = a2 + (uint16_t)(idx << 5);
        for (int k = 0; k < 8; k++) {
            uint32_t v = rd32(m, a3);
            a3 += 4u;
            v = (v >> 16) | (v << 16);
            wr32(m, a1, v);
            a1 += 4u;
        }
        count--;
    } while (count >= 0);
}

/* ROM 0x005B0C */
void Gfx_InitLineRam(const f3mem_t *m)
{
    uint32_t a0 = 0x00005BF8u;
    uint32_t a1 = 0x00624000u;
    uint16_t d0 = rd8(m, F3_A5 - 0x5D58u) ? 0u : 0x30u;
    a1 += d0;
    wr16(m, F3_A5 - 0x5D56u, d0);
    for (int r = 0; r < 8; r++) {
        for (int k = 0; k < 4; k++) {
            a1 = fill_lineram_run(m, a1, rd16(m, a0));
            a0 += 2u;
        }
        a1 += 0x800u;
    }
    a1 = 0x00620000u;
    d0 = 0x100Fu;
    for (int r = 0; r < 8; r++) {
        for (int i = 0; i < 0x100; i++) {
            wr16(m, a1, d0);
            a1 += 2u;
        }
        d0 = (uint16_t)(d0 + 0x0400u);
    }
    if (rd16(m, F3_A5 - 0x5D58u) == 0u) {
        wr16(m, 0x00626000u, 0x0800u);
        wr16(m, 0x006275FEu, (uint16_t)(rd16(m, 0x006275FEu) | 0x0080u));
        wr16(m, 0x006257FEu, 0x6E00u);
        wr16(m, 0x006247FEu, (uint16_t)(rd16(m, 0x006247FEu) | 0x8000u));
    } else {
        wr16(m, 0x006261FEu, 0x0800u);
        wr16(m, 0x00627400u, (uint16_t)(rd16(m, 0x00627400u) | 0x0080u));
        wr16(m, 0x00625600u, 0x6E00u);
        wr16(m, 0x00624600u, (uint16_t)(rd16(m, 0x00624600u) | 0x8000u));
    }
}

/* ROM 0x13D11E */
void Stage_DrawBanner13D11E(const f3mem_t *m)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);
    uint32_t a0 = rd32(m, 0x0013D558u + (int32_t)idx * 4);
    Gfx_UploadTextTiles(m, a0, rd8(m, 0x00010C05u));
    Gfx_DrawTilemapBlock(m, 0x00010C00u, 0x0061C024u, 0u);
}

/* ROM 0x13D7A0 */
void Stage_NameEntry13D7A0(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d7)
{
    int16_t idx = (int16_t)rd16(m, a0 + 0x22u);
    uint32_t a2 = rd32(m, 0x0013D916u + (int32_t)idx * 4);
    int16_t d1 = 8;
    int16_t d2 = (int16_t)((int16_t)rd32(m, a0 + 0x48u) * 2 + 0x0D);
    uint8_t d3 = rd8(m, a2++);
    uint32_t dst = 0x0061C000u + (int32_t)(int16_t)((d2 << 7) + (d1 << 1));
    uint32_t d0 = Text_DrawString(m, a2, dst, d3);
    a2 = d0 + 0x2Au + (int32_t)(int16_t)rd16(m, a0 + 0x3Eu) * 2;
    if ((d7 & 1u) == 0u) {
        wr16(m, a2, (uint16_t)((rd16(m, a2) & 0x81FFu) | (5u << 9)));
    } else {
        wr16(m, a2, (uint16_t)((rd16(m, 0x000106B6u) & 0x81FFu) | (8u << 9)));
    }
    uint32_t inp = rd32(m, a0 + 0x44u);
    int skip_char = 0;
    if ((rd8(m, inp) & 0x08u) == 0u) {
        int16_t t = (int16_t)rd16(m, a0 + 6u);
        if (t < 0x14) {
            t++;
            wr16(m, a0 + 6u, (uint16_t)t);
            if (t > 1) skip_char = 1;
        }
        if (!skip_char) {
            wr16(m, a0 + 0x3Au, (uint16_t)(rd16(m, a0 + 0x3Au) + 1u));
        }
    } else if ((rd8(m, inp) & 0x04u) == 0u) {
        int16_t t = (int16_t)rd16(m, a0 + 6u);
        if (t < 0x14) {
            t++;
            wr16(m, a0 + 6u, (uint16_t)t);
            if (t > 1) skip_char = 1;
        }
        if (!skip_char) {
            wr16(m, a0 + 0x3Au, (uint16_t)(rd16(m, a0 + 0x3Au) - 1u));
        }
    } else {
        wr16(m, a0 + 6u, 0u);
    }
    if (!skip_char) {
        int16_t ch = (int16_t)rd16(m, a0 + 0x3Au);
        if (ch < 0) ch += 0x25;
        if (ch > 0x24) ch -= 0x25;
        wr16(m, a0 + 0x3Au, (uint16_t)ch);
        int16_t pos = (int16_t)rd16(m, a0 + 0x3Eu);
        wr8(m, a1 + 6u + (int32_t)pos, rd8(m, 0x0013D926u + ch));
    }
    if ((int16_t)d7 > 0) {
        if ((rd8(m, inp + 1u) & 0x20u) == 0u) {
            int16_t pos = (int16_t)rd16(m, a0 + 0x3Eu) - 1;
            if (pos < 0) pos = 0;
            wr16(m, a0 + 0x3Eu, (uint16_t)pos);
        }
        if ((rd8(m, inp + 1u) & 0x10u) == 0u) {
            int16_t pos = (int16_t)rd16(m, a0 + 0x3Eu) + 1;
            wr16(m, a0 + 0x3Eu, (uint16_t)pos);
            if (pos >= 3) {
                wr32(m, a0 + 0x48u, 0xFFFFFFFFu);
            }
        }
    }
    if (rd32(m, a1 + 6u) == 0x4B4B4B00u || rd32(m, a1 + 6u) == 0x53455800u) {
        wr32(m, a1 + 6u, 0x41414100u);
    }
}

/* ROM 0x13D94C */
void Stage_DrawHighScores13D94C(const f3mem_t *m)
{
    uint32_t lram = 0x00626200u + (int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (int i = 0; i < 0xE8; i++) {
        wr16(m, lram + (uint32_t)i * 2u, 0x0CFCu);
    }
    Text_DrawString(m, 0x0013DAAAu, 0x0061C41Cu, 5u);
    Text_DrawString(m, 0x0013DAB6u, 0x0061C590u, 8u);
    uint32_t a0 = 0x00406CD4u;
    int16_t d1 = 1;
    int16_t d2 = 0x0D;
    for (; d1 <= 5; d1++, d2 += 2, a0 += 0x0Cu) {
        int16_t d3 = 0x0B;
        uint32_t dst = 0x0061C000u + (int32_t)(int16_t)((d2 << 7) + (d3 << 1));
        uint32_t d0 = Gfx_DrawHex(m, (uint32_t)(uint16_t)d1, 1u, dst, 0x0Au);
        d0 += 2u;
        d0 = Gfx_DrawDecimal(m, rd32(m, a0), 8u, d0, 7u);
        d0 += 2u;
        int16_t col = 0;
        int16_t d4 = 0;
        uint32_t a1 = 0x0013D5C8u;
        int8_t limit = (int8_t)rd8(m, a0 + 4u);
        do {
            d4++;
            uint16_t d5 = (d4 < limit) ? 6u : 4u;
            d0 = Text_DrawString(m, a1 + (int32_t)col * 2, d0, d5);
            if (col == 0x15 || col == 0x1B) {
                d0 = Text_DrawString(m, 0x0013DAD1u, d0, d5);
                d0 -= 2u;
            }
            uint16_t bit = (rd8(m, a0 + 5u) >> 2) & 1u;
            col = (int16_t)(col + d4 + bit);
        } while (d4 < 7 && d4 < limit);
        d4 = (int16_t)(d4 - 9);
        d0 -= (int32_t)(int16_t)(d4 + d4);
        Text_DrawString(m, a0 + 6u, d0, 7u);
    }
}

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

/* ROM 0x001C90: upload tilemap scroll registers (0x660000..0x66001E) + sprite scroll */
void Video_UploadScroll(const f3mem_t *m)
{
    if (m->spr)
        Video_SetSpriteScroll(m);
}

/* ROM 0x001C90 at register level, for calls whose continuation is compared
 * (0x01010A): the scroll arithmetic itself only feeds the write-only video
 * registers 0x660000-0x66001E, which are not modelled, but the routine leaves
 * D0, D1, D2 and A0 behind. At 0x001D88: A0 = 0x600020 (from the 0x001232
 * tail), D1 = 0x0A and D2 = 0x09 (the two moveq), and D0 keeps the high half
 * of the 0x001D34/0x001D38 `asr.l #9` of the long at A5-0x5D64 with its low
 * word replaced twice (0x001D6A, 0x001D76: the word at A5-0x5D60 / A5-0x5D5C
 * plus the 9th/10th word of the 0x402242 table), then once more by
 * 0x00124C (the word at A5-0xCA8) in Video_SetSpriteScroll.
 *
 * The eight longs at A5-0x5D80..-0x5D64 are negated at 0x001CAA (only when
 * the word at A5-0x5D58 is set) and negated back at 0x001D4A, so they end as
 * they started; only the negated value feeds the arithmetic, which is what is
 * modelled here (no RAM write). */
void Video_UploadScroll_Regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t v = rd32(m, a5 - 0x5D64u);              /* 0x001D34 move.l */
    if (rd16(m, a5 - 0x5D58u) != 0u)                 /* 0x001CAA neg.l x8 */
        v = 0u - v;                                  /* ... undone 0x001D4A */
    uint32_t d0 = (uint32_t)((int32_t)v >> 9);       /* 0x001D38 asr.l #9 */
    d0 = (d0 & 0xFFFF0000u)
       | (uint16_t)(rd16(m, a5 - 0x5D60u) + rd16(m, 0x00402252u)); /* 0x001D6A */
    d0 = (d0 & 0xFFFF0000u)
       | (uint16_t)(rd16(m, a5 - 0x5D5Cu) + rd16(m, 0x00402254u)); /* 0x001D76 */
    if (m->spr)                                     /* 0x001D82 jsr 0x001232 */
        Video_SetSpriteScroll(m);
    r->d[0] = (d0 & 0xFFFF0000u) | rd16(m, a5 - 0x0CA8u); /* 0x00124C */
    r->d[1] = 0x0000000Au;                           /* 0x001C9E moveq #0xa */
    r->d[2] = 0x00000009u;                           /* 0x001CA0 moveq #9 */
    r->a[0] = 0x00600020u;                           /* 0x001232 lea */
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

/* ROM 0x0053B2 */
void Gfx_DrawCompressedTilemap(const f3mem_t *m, uint32_t src, uint32_t dst, uint32_t flags)
{
    uint32_t a0 = src;
    int16_t rows = (int16_t)((uint16_t)rd8(m, a0++) - 1u);
    int16_t cols = (int16_t)((uint16_t)rd8(m, a0++) - 1u);
    uint32_t d2 = (flags >> 30) & 3u;
    uint16_t d5_lo = (uint16_t)(dst & 0xFFFFu);
    uint16_t d6_lo = d5_lo;
    int16_t row_step = 0x0080;
    int16_t col_step = 4;
    if (d2 == 1u) {
        d6_lo = (uint16_t)(d6_lo + (uint16_t)(cols << 2));
        col_step = -4;
    } else if (d2 == 2u) {
        row_step = -0x0080;
        d5_lo = (uint16_t)(d5_lo + (uint16_t)(rows << 7));
    } else if (d2 == 3u) {
        row_step = -0x0080;
        d5_lo = (uint16_t)(d5_lo + (uint16_t)(rows << 7));
        d6_lo = (uint16_t)(d6_lo + (uint16_t)(cols << 2));
        col_step = -4;
    }
    uint32_t d3 = flags & 0xFE000000u;
    int16_t run_cnt = 0;
    uint8_t run_op = 0;
    uint32_t run_val = 0;
    int16_t r = rows;
    do {
        uint16_t d4 = d6_lo;
        uint32_t row_base = (dst & 0xFFFFF000u) + (d5_lo & 0x0F80u);
        int16_t c = cols;
        do {
            uint16_t off = d4 & 0x007Cu;
            if (run_cnt == 0) {
                uint16_t tok = rd16(m, a0);
                a0 += 2u;
                if (tok == 0u) {
                    wr32(m, row_base + off, rd32(m, a0) ^ d3);
                    a0 += 4u;
                } else {
                    run_op = (uint8_t)(tok >> 8);
                    run_cnt = (int16_t)(tok & 0xFFu);
                    if (run_op == 1u) {
                        wr32(m, row_base + off, 0u);
                    } else {
                        run_val = rd32(m, a0) ^ d3;
                        a0 += 4u;
                        wr32(m, row_base + off, run_val);
                        if (run_op == 3u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val + 1u);
                        else if (run_op == 4u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val - 1u);
                    }
                }
            } else {
                run_cnt--;
                if (run_op == 1u) {
                    wr32(m, row_base + off, 0u);
                } else {
                    wr32(m, row_base + off, run_val);
                    if (run_op == 3u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val + 1u);
                    else if (run_op == 4u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val - 1u);
                }
            }
            d4 = (uint16_t)(d4 + col_step);
            c--;
        } while (c >= 0);
        d5_lo = (uint16_t)(d5_lo + row_step);
        r--;
    } while (r >= 0);
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
    Gfx_ClearTextLayerFrom(m, 0x0061C000u);
}

void Gfx_ClearTextLayerFrom(const f3mem_t *m, uint32_t addr)
{
    for (; addr < 0x0061E000u; addr += 4u)
        wr32(m, addr, 0u);
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
    Gfx_ClearPlayfields(m);
}

void Gfx_ClearPlayfields(const f3mem_t *m)
{
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

/* ROM 0x005B0C as its caller sees it: the RAM rebuild plus the registers it
 * leaves. A0 ends on the byte after the 0x5BF8 row table (8 rows x 4 words),
 * A1 on the byte after the 0x620000 row table (8 rows x 0x100 words), D3 and
 * D1 on -1 after their DBF loops, D0 = the 0x624000-derived value of
 * 0x005B50..0x005B62 whose low word the 0x005B8A/0x005BD6 `move.w <control>,D0`
 * + `ori.w #0x8000` replaces, and D2 is rotated by 4 eight times = unchanged.
 * D0's high word is 0x0018 for every path (0x00624000 >>8 >>4 <<8 <<2, then
 * `andi.w #0x7fff` and eight `addi.w #0x400` that never carry out of the low
 * word). */
void Gfx_InitLineRam_Regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = 0x00624000u;                                   /* 0x005B50 */
    d0 >>= 8;                                                    /* 0x005B56 */
    d0 >>= 4;                                                    /* 0x005B58 */
    d0 <<= 8;                                                    /* 0x005B5A */
    d0 <<= 2;                                                    /* 0x005B5C */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x000Fu); /* 0x005B5E */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 & 0x7FFFu); /* 0x005B62 */
    if (rd16(m, F3_A5 - 0x5D58u) == 0u)                          /* 0x005B7A tst.w */
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)(rd16(m, 0x006247FEu) | 0x8000u); /* 0x005BA2 */
    else
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)(rd16(m, 0x00624600u) | 0x8000u); /* 0x005BD6 */

    Gfx_InitLineRam(m);
    r->d[0] = d0;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0xFFFFu;                 /* 0x005B6C dbf D1 */
    r->d[3] = (r->d[3] & 0xFFFF0000u) | 0xFFFFu;                 /* 0x005B76 dbf D3 */
    r->a[0] = 0x00005C38u;                                       /* 0x005BF8 + 8*4*2 */
    r->a[1] = 0x00621000u;                                       /* 0x00620000 + 8*0x100*2 */
}

/* ROM 0x13D11E */
void Stage_DrawBanner13D11E(const f3mem_t *m)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);
    uint32_t a0 = rd32(m, 0x0013D558u + (int32_t)idx * 4);
    Gfx_UploadTextTiles(m, a0, rd8(m, 0x00010C05u));
    Gfx_DrawTilemapBlock(m, 0x00010C00u, 0x0061C024u, 0u);
}

/* ROM 0x13D11E with the register leftovers its callers keep: 0x005A86 saves
 * nothing, so A0-A3/D0-D2 hold its end state (A1 = end of the tile block,
 * A3 = end of the last tile source, D2 = the last swapped long, D0/D1 = -1
 * after their DBF); 0x0055C8 keeps every register. D0.b at entry is the tile
 * slot from 0x10C05, used as `lsl.w #5,D0 ; adda.w D0,A1`. */
void Stage_DrawBanner13D11E_Regs(const f3mem_t *m, f3regs_t *r)
{
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);              /* 0x13D124 */
    uint32_t a0 = rd32(m, 0x0013D558u + (int32_t)idx * 4);         /* 0x13D128 */
    uint16_t d0 = rd8(m, 0x00010C05u);                             /* 0x13D12C..0x13D12E */
    uint32_t a1 = 0x0061E000u                                      /* 0x005A86 */
                + (uint32_t)(int32_t)(int16_t)(uint16_t)(d0 << 5); /* 0x005A8C..0x005A8E */
    uint32_t a2 = 0x0001120Au;                                     /* 0x005A90 */
    int16_t count = (int16_t)rd16(m, a0);                          /* 0x005A96 */
    uint16_t d1 = 0;
    uint32_t d2 = 0, a3 = 0;
    a0 += 2u;
    do {
        d1 = (uint16_t)(rd16(m, a0) << 5);                          /* 0x005A98..0x005A9A */
        a0 += 2u;
        a3 = a2 + (uint32_t)(int32_t)(int16_t)d1;                   /* 0x005A9C lea (A2,D1.w),A3 */
        for (int k = 0; k < 8; k++) {                               /* 0x005AA4 dbf #7 */
            d2 = rd32(m, a3);
            a3 += 4u;
            d2 = (d2 >> 16) | (d2 << 16);                           /* 0x005AA6 swap D2 */
            wr32(m, a1, d2);
            a1 += 4u;
        }
        count--;                                                    /* 0x005AAE dbf D0 */
    } while (count >= 0);
    r->a[0] = a0; r->a[1] = a1; r->a[2] = a2; r->a[3] = a3;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)count;            /* D0.w = -1 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0xFFFFu;                    /* 0x005AAA dbf D1 */
    r->d[2] = d2;
    Gfx_DrawTilemapBlock(m, 0x00010C00u, 0x0061C024u, 0u);         /* 0x13D13A */
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

/* ROM 0x16A9BC (with ROM 0x16ABA2) */
void Stage_UploadBgTilemaps(const f3mem_t *m)
{
    static const struct { uint16_t flag_off, clr1, clr2, p1, p2; uint32_t pf_base; } layers[4] = {
        { 0x4284u, 0x4204u, (uint16_t)(-0x5DA8), 0x4264u, 0x4268u, 0x00610000u },
        { 0x4285u, 0x420Cu, (uint16_t)(-0x5DA0), 0x426Cu, 0x4270u, 0x00611000u },
        { 0x4286u, 0x4214u, (uint16_t)(-0x5D98), 0x4274u, 0x4278u, 0x00612000u },
        { 0x4287u, 0x421Cu, (uint16_t)(-0x5D90), 0x427Cu, 0x4280u, 0x00613000u },
    };
    for (int l = 0; l < 4; l++) {
        if ((rd8(m, F3_A5 + layers[l].flag_off) & 1u) == 0u) continue;
        wr16(m, F3_A5 + layers[l].clr1, 0u);
        wr16(m, F3_A5 + (int16_t)layers[l].clr2, 0u);
        uint32_t a1 = rd32(m, F3_A5 + layers[l].p1);
        uint32_t a2 = rd32(m, F3_A5 + layers[l].p2);
        for (int16_t d6 = 3; d6 >= 0; d6--) {
            for (int16_t d7 = 3; d7 >= 0; d7--) {
                int16_t idx = (int16_t)((d6 << 2) + d7);
                int16_t t = (int16_t)rd16(m, a1 + (int32_t)idx * 2);
                uint32_t src = rd32(m, a2 + (int32_t)(t << 2));
                int16_t d0 = (int16_t)((d6 & 3) << 3);
                int16_t d1 = (int16_t)(d7 << 3);
                uint32_t dst = layers[l].pf_base + (int16_t)(d1 * 0x80 + (d0 << 2));
                Gfx_DrawCompressedTilemap(m, src, dst, 0u);
            }
        }
    }
}

/* ROM 0x00117E: clear both sprite RAM banks (0x600030 and 0x608030) and reset cursors */
void Sprite_ClearAll(const f3mem_t *m)
{
    uint32_t a0 = 0x00600030u;
    uint32_t a1 = 0x00608030u;
    for (int i = 0; i <= 0x3FB; i++) {
        wr16(m, a0, 0u);
        wr16(m, a0 + 2u, 0xFFFFu);
        wr16(m, a1, 0u);
        wr16(m, a1 + 2u, 0xFFFFu);
        a0 += 0x10u;
        a1 += 0x10u;
    }
    a0 = 0x00600030u;
    a1 = 0x00608030u;
    for (int i = 0; i <= 0x3FB; i++) {
        wr16(m, a0 + 4u, 0x0FF0u);
        wr16(m, a0 + 6u, 0x0FF0u);
        wr16(m, a0 + 8u, 0u);
        wr16(m, a0 + 0xAu, 0u);
        wr16(m, a0 + 0xCu, 0u);
        wr16(m, a0 + 0xEu, 0u);
        wr16(m, a1 + 4u, 0x0FF0u);
        wr16(m, a1 + 6u, 0x0FF0u);
        wr16(m, a1 + 8u, 0u);
        wr16(m, a1 + 0xAu, 0u);
        wr16(m, a1 + 0xCu, 0u);
        wr16(m, a1 + 0xEu, 0u);
        a0 += 0x10u;
        a1 += 0x10u;
    }
    wr32(m, F3_A5 - 0x0CA0u, 0x00600310u);
    wr32(m, F3_A5 - 0x0C9Cu, 0x00600310u);
    wr32(m, F3_A5 - 0x0C98u, 0x00608310u);
    wr32(m, F3_A5 - 0x0CACu, 0u);
    wr32(m, F3_A5 - 0x0CA8u, 0u);
    wr16(m, F3_A5 - 0x0CA4u, (uint16_t)(rd16(m, F3_A5 - 0x0CA4u) & 0xFFEDu));
}

/* ROM 0x19F38A: clear all layers and draw the 40x29 test-mode grid on text RAM (0x61C000) */
void Gfx_DrawTestGrid19F38A(const f3mem_t *m)
{
    uint32_t a2 = 0x0061C000u;
    Gfx_ClearAllLayers(m);

    for (uint32_t d3 = 0; d3 < 0x0Eu; d3++) {
        for (uint32_t d2 = 0; d2 < 0x28u; d2++) {
            uint32_t off = (d3 << 7) + (d2 << 1);
            uint32_t tbl = (d3 < 2u || d2 < 2u || d2 > 0x25u) ? 0x001A1E9Cu : 0x001A1E94u;
            uint32_t idx = ((d2 & 1u) << 1) + ((d3 & 1u) << 2);
            wr16(m, a2 + off, rd16(m, tbl + idx));
        }
    }

    for (uint32_t d2 = 0; d2 < 0x28u; d2++) {
        uint32_t off = (0x0Eu << 7) + (d2 << 1);
        wr16(m, a2 + off, rd16(m, 0x001A1EA4u + ((d2 & 1u) << 1)));
    }

    for (uint32_t d3 = 0x0Fu; d3 < 0x1Du; d3++) {
        for (uint32_t d2 = 0; d2 < 0x28u; d2++) {
            uint32_t off = (d3 << 7) + (d2 << 1);
            uint32_t tbl = (d3 > 0x1Au && (d2 < 2u || d2 > 0x25u)) ? 0x001A1E9Cu : 0x001A1E94u;
            uint32_t idx = ((d2 & 1u) << 1) + (((d3 + 1u) & 1u) << 2);
            wr16(m, a2 + off, rd16(m, tbl + idx));
        }
    }
}

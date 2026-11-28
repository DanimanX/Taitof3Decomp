/* ROM 0x0B7A54: (recipe capture fnk_, work/zcap/recipes15.tsv): draw the 16-glyph hex legend of the
 * sound/option editor twice (at row D2 and row D2+3) and an attribute ramp bar
 * underneath, all in text RAM 0x61C000.
 *
 * Arguments are registers: D1.w = column, D2.w = row. Both legend rows print
 * the "0123456789ABCDEF" string at 0x0B7B16 with attribute 7 at
 * 0x61C000 + 2*D1 + 0x80*D2 and 0x61C000 + 2*D1 + 0x80*(D2+3); the bar starts
 * at 0x61C000 + 0x80*D2 + 0x80 + 2*D1 and writes two runs of 0x10 words, the
 * second 0x180 bytes (three text rows) lower. The attribute word starts at
 * 0x2807 and its low byte counts 7..0x0E before wrapping back to 7 with 0x200
 * added to the word - so each group of eight cells steps the colour.
 *
 * movem.l {D0 D1 D2 D3 A0 A1},-(SP) / (SP)+ leaves every register unchanged. */
#include "fn_auto.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (0x005666) */

/* 0x0B7A5E..0x0B7A82 and the same block with D2+3: 0x0B7A86..0x0B7AB0. */
static void legend_row(const f3mem_t *m, uint16_t x, uint16_t y)
{
    uint32_t dst = 0x0061C000u +
                   (uint32_t)(int32_t)(int16_t)(uint16_t)(x + x) +
                   (uint32_t)(int32_t)(int16_t)(uint16_t)(y << 7);
    (void)Text_DrawString(m, 0x000B7B16u, dst, 7u);
}

int Fn_0B7A54(const f3mem_t *m, f3regs_t *r)
{
    const uint16_t x = (uint16_t)r->d[1];
    const uint16_t y = (uint16_t)r->d[2];
    uint32_t a0;
    uint16_t d1;
    unsigned i;

    legend_row(m, x, y);                                   /* 0x0B7A58..0x0B7A82 */
    legend_row(m, x, (uint16_t)(y + 3u));                   /* 0x0B7A86..0x0B7AB0 */

    /* 0x0B7AB4..0x0B7ABE: A0 = 0x61C000 + 0x80*(y) + 0x80 + 2*x. */
    a0 = 0x0061C000u + (uint32_t)(int32_t)(int16_t)(uint16_t)(y << 7);
    a0 += 0x80u + 2u * (uint32_t)(int32_t)(int16_t)x;
    const uint32_t start = a0;

    /* 0x0B7AC6..0x0B7ACE: 0x14 << 8 << 1 | 7 = 0x2807. */
    d1 = 0x2807u;
    for (i = 0; i < 0x10u; ++i) {                          /* 0x0B7AD6..0x0B7AEA */
        wr16(m, a0, d1);
        a0 += 2u;
        d1 = (uint16_t)((d1 & 0xFF00u) | (uint8_t)((d1 & 0xFFu) + 1u)); /* addq.b #1 */
        if ((int8_t)d1 > 0x0E) {                           /* 0x0B7ADA cmpi.b #0xE */
            d1 = (uint16_t)((d1 & 0xFF00u) | 0x07u);       /* 0x0B7AE2 move.b #7 */
            d1 = (uint16_t)(d1 + 0x200u);                  /* 0x0B7AE6 addi.w #0x200 */
        }
    }

    a0 = start + 0x180u;                                   /* 0x0B7AEE..0x0B7AF0 */
    for (i = 0; i < 0x10u; ++i) {                          /* 0x0B7AF8..0x0B7B0C */
        wr16(m, a0, d1);
        a0 += 2u;
        d1 = (uint16_t)((d1 & 0xFF00u) | (uint8_t)((d1 & 0xFFu) + 1u));
        if ((int8_t)d1 > 0x0E) {
            d1 = (uint16_t)((d1 & 0xFF00u) | 0x07u);
            d1 = (uint16_t)(d1 + 0x200u);
        }
    }

    return 0;                                              /* 0x0B7B10 movem restore, rts */
}

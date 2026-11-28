#include "fn_auto.h"
/* ROM 0x091E2C: (palette capture fnp_): pick one of six palette
 * blocks from the stage byte A5-0xC4E and copy it 32 longs per row into
 * palette RAM at 0x440000. The existing capture checks register preservation.
 *
 * 0x0014E8 (table walk) and 0x001520 (block copy) are inlined below.
 * Block table 0x16890: {src.l, rows.l} per index; the six used indexes are
 * 0..5 (block pointers 0x15AEC, 0x15C9C, 0x15EF4, 0x15FEA, 0x1609E, 0x16164
 * with 71, 99, 40, 29, 32 and 64 rows). Each row is {n.w, src_off.w,
 * dst_off.w} at the block pointer and copies n longs from 0x16900+src_off to
 * 0x440000+dst_off (n = 0x20 for every row of every block). */

#define COLOR_TBL 0x00016900u   /* base of the {col*2,char}-style colour words */

static void copy_block(const f3mem_t *m, uint32_t a1, uint16_t rows, uint32_t a2)
{
    uint16_t d3 = rows;
    for (;;) {                                        /* 0x001530 */
        uint16_t d2 = (uint16_t)(rd16(m, a2) - 1u);   /* move.w (A2)+,D2w ; subq.w #1 */
        a2 += 2u;
        uint32_t d0 = rd16(m, a2);                    /* moveq #0,D0 ; move.w (A2)+,D0w */
        a2 += 2u;
        uint16_t d1 = rd16(m, a2);                    /* move.w (A2)+,D1w */
        a2 += 2u;
        for (;;) {                                    /* 0x00153A */
            wr32(m, F3_PAL_BASE + (uint32_t)(int32_t)(int16_t)d1, rd32(m, a1 + d0));
            d0 += 4u;                                 /* 0x001540 addq.l #4,D0 */
            d1 = (uint16_t)(d1 + 4u);                 /* 0x001542 addq.w #4,D1w */
            if (d2-- == 0u)                           /* 0x001544 dbf D2w */
                break;
        }
        if (d3-- == 0u)                               /* 0x001548 dbf D3w */
            break;
    }
}

int Fn_091E2C(const f3mem_t *m, f3regs_t *r)
{
    /* 0x0014E8: index = ((1 + stage byte) & 0xFF) % 6 through divu.w #6/swap */
    uint32_t d0 = 1u;
    d0 = (d0 & 0xFFFFFF00u) | (uint8_t)(d0 + rd8(m, F3_A5 - 0x0C4Eu)); /* 0x091E32 */
    d0 &= 0xFFu;                                      /* 0x0014F4 andi.l #0xFF */
    uint16_t idx = (uint16_t)(d0 % 6u);               /* divu.w #6,D0 ; swap D0 */

    uint32_t a2 = rd32(m, 0x00016890u + (uint32_t)idx * 8u);      /* 0x001520 */
    uint32_t d3 = rd32(m, 0x00016890u + (uint32_t)idx * 8u + 4u); /* 0x001524 */
    if ((int32_t)d3 >= 0)                             /* 0x001528 bmi 0x00154C */
        copy_block(m, COLOR_TBL, (uint16_t)d3, a2);

    /* 0x091E3C movem.l (SP)+ restores D0-D7/A0-A4: every register is as it
     * entered, so r is left untouched. */
    (void)r;
    return 0;                                         /* 0x091E40 rts */
}

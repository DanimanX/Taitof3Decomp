#include "fn_auto.h"
/* ROM 0x19F4A6: clear every tile/line layer (0x005A12) and then fill the
 * text-RAM glyph grid at 0x61C088 with tile codes: for D4 = 0..6 (row block),
 * D5 = 0..31 (column, word step 2) and D3 = 0..3 (cell) it writes
 * ((D6 << 9) + D2 + 0x16).w at 0x61C088 + ((D3 + D4*4) << 7) + D5*2, where D2
 * counts 0..15 and D6 (starting at 0x20) increments every 16 cells. D2-D6 are
 * saved; D0, D1 and A0 end holding the last index arithmetic. */
#include "../video/sprite.h"

int Fn_19F4A6(const f3mem_t *m, f3regs_t *r)
{
    Gfx_ClearAllLayers(m);                        /* 0x19F4AA jsr 0x005A12 */

    uint32_t d0, d1, d2 = 0u, d3, d4 = 0u, d5, d6 = 0x20u;
    do {
        d5 = 0u;
        do {
            d3 = 0u;
            do {
                d1 = 9u;                          /* 0x19F4BA moveq #9,D1 */
                d0 = d6 << d1;                    /* 0x19F4BC lsl.l D1,D0 */
                d0 = (d0 + d2) & 0xFFFFFFFFu;     /* 0x19F4C0 add.l D2,D0 */
                d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0016u); /* 0x19F4C2 */
                r->a[0] = d3 + d4 * 4u;           /* 0x19F4C6..0x19F4C8 */
                d1 = (r->a[0] << 7) & 0xFFFFFFFFu; /* 0x19F4CC..0x19F4CE */
                r->a[0] = d1;                     /* 0x19F4D0 movea.l D1,A0 */
                wr16(m, 0x0061C088u + r->a[0] + d5 * 2u, (uint16_t)d0); /* 0x19F4D2 */
                d3 = (d3 + 1u) & 0xFFFFFFFFu;     /* 0x19F4DA addq.l #1,D3 */
                d0 = (d3 - 4u) & 0xFFFFFFFFu;     /* 0x19F4DC..0x19F4DE */
            } while ((int32_t)d0 < 0);            /* 0x19F4E0 blt.b 0x19F4BA */
            d2 = (d2 + 1u) & 0xFFFFFFFFu;         /* 0x19F4E2 addq.l #1,D2 */
            d0 = 0x0000000Fu;                     /* 0x19F4E4 moveq #0xF,D0 */
            if ((int32_t)d2 >= (int32_t)d0) {     /* 0x19F4E6 cmp.l D0,D2 ; 0x19F4E8 blt */
                d6 = (d6 + 1u) & 0xFFFFFFFFu;     /* 0x19F4EA addq.l #1,D6 */
                d2 = 0u;                          /* 0x19F4EC moveq #0,D2 */
            }
            d5 = (d5 + 1u) & 0xFFFFFFFFu;         /* 0x19F4EE addq.l #1,D5 */
            d0 = 0x00000020u;                     /* 0x19F4F0 moveq #0x20,D0 */
        } while ((int32_t)d5 < (int32_t)d0);      /* 0x19F4F2 cmp.l D0,D5 ; blt.b */
        d4 = (d4 + 1u) & 0xFFFFFFFFu;             /* 0x19F4F6 addq.l #1,D4 */
        d0 = (d4 - 7u) & 0xFFFFFFFFu;             /* 0x19F4F8..0x19F4FA */
    } while ((int32_t)d0 < 0);                    /* 0x19F4FC blt.b 0x19F4B6 */

    r->d[0] = d0;                                 /* 0 after the final subq */
    r->d[1] = d1;
    return 0;                                     /* 0x19F502 rts */
}

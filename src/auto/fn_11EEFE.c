/* ROM 0x11EEFE: stage/boss scroll-in counter tick (A0 = object).
 * Increments A5+0x30D2 by 1.0 (16.16) and jitters if A5+0x1D58 bit 1 set;
 * re-arms 0x11EEFE while < 0xF0, otherwise sets A5+0x30EE bitfields, arms
 * +0x06=10, +0x20=0x8000, +0x42=0x80, clears +0x18, and transitions to 0x11EF66. */
#include "fn_auto.h"

int Fn_11EEFE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x00010000u); /* 0x11EEFE */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x11EF06 */
        d0 = Rng_NextMem(m);                              /* 0x11EF0E */
        uint16_t d0w = (uint16_t)(((uint16_t)d0 & 0x0003u) - 1u);
        d0 = (d0 & 0xFFFF0000u) | d0w;
        if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0)   /* 0x11EF1A..0x11EF20 */
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) - d0w));
        else
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + d0w));
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) >= 0x00F0) {    /* 0x11EEEC..0x11EEF2 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* bfset {0xC:1} */
        wr16(m, obj + 0x06u, 0x000Au);                    /* 0x11EF34 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x08u)); /* bfset {0x4:1} */
        if (rd8(m, F3_A5 + 0x3260u) == 0u)                /* 0x11EF40 */
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* bfset {0x3:1} */
        wr16(m, obj + 0x20u, 0x8000u);                    /* 0x11EF4E */
        wr16(m, obj + 0x42u, 0x0080u);                    /* 0x11EF54 */
        wr32(m, obj + 0x18u, 0u);                         /* 0x11EF5A */
        wr32(m, obj, 0x0011EF66u);                        /* 0x11EF5E */
    } else {
        wr32(m, obj, 0x0011EEFEu);                        /* 0x11EEF6 */
    }

    r->d[0] = d0;
    return 0;
}

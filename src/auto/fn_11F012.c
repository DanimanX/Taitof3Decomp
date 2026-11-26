/* ROM 0x11F012: camera shake / boss phase wait tick (A0 = object).
 * When A5+0x1D58 bit 1 is set, rolls Rng_NextMem (0x001D8A), takes (D0 & 3)-1
 * and nudges camera X (A5+0x30D2.w) toward 0xA0 (sub if > 0xA0 else add).
 * Decrements +0x06 and re-arms 0x11F012 while > 0; otherwise sets A5+0x30F2 = 1,
 * +0x06 = 0x1A4, and arms 0x11F052. */
#include "fn_auto.h"

int Fn_11F012(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x11F012..0x11F018 */
        uint32_t rng = Rng_NextMem(m);                    /* 0x11F01A -> 0x001D8A */
        uint16_t d0w = (uint16_t)((rng & 0x0003u) - 1u);  /* 0x11F020..0x11F024 */
        r->d[0] = (rng & 0xFFFF0000u) | d0w;
        if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0)   /* 0x11F026..0x11F02C */
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) - d0w)); /* 0x11F02E */
        else
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + d0w)); /* 0x11F034 */
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x11F038 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                        /* 0x11F03C */
        wr32(m, obj, 0x0011F012u);                        /* 0x11F00A */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 0x01u);                       /* 0x11F03E */
    wr16(m, obj + 0x06u, 0x01A4u);                        /* 0x11F044 */
    wr32(m, obj, 0x0011F052u);                            /* 0x11F04A */
    return 0;
}

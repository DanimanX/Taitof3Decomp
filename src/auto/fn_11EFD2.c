/* ROM 0x11EFD2: screen-shake / horizontal jitter countdown tick (A0 = object).
 * When A5+0x1D58 bit 1 is set, steps Rng_NextMem (0x001D8A) and nudges
 * A5+0x30D2 toward 0xA0 by (rng & 3) - 1. Decrements +0x06.w; while > 0 re-arms
 * 0x11EFD2, else clears A5+0x30EE bit 4 (0x10), sets +0x06 = 0x3C, and
 * transitions to 0x11F012. */
#include "fn_auto.h"

int Fn_11EFD2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {                 /* 0x11EFD2..0x11EFD8 */
        uint32_t rng = Rng_NextMem(m);                             /* 0x11EFDA (0x001D8A) */
        uint16_t d0w = (uint16_t)((rng & 0x0003u) - 1u);           /* 0x11EFE0..0x11EFE4 */
        r->d[0] = (rng & 0xFFFF0000u) | d0w;
        int16_t x = (int16_t)rd16(m, F3_A5 + 0x30D2u);             /* 0x11EFE6 */
        if (x <= 0x00A0)                                           /* 0x11EFEC */
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(x + (int16_t)d0w)); /* 0x11EFF4 */
        else
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(x - (int16_t)d0w)); /* 0x11EFEE */
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);          /* 0x11EFF8 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                        /* 0x11EFFC */
        wr32(m, obj, 0x0011EFD2u);                                 /* 0x11EFCA */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x10u)); /* 0x11EFFE */
    wr16(m, obj + 0x06u, 0x003Cu);                                 /* 0x11F004 */
    wr32(m, obj, 0x0011F012u);                                     /* 0x11F00A */
    return 0;
}

/* ROM 0x17B1A6: cycling boss palette upload tick (A0 = object).
 * Masks (A5+0x878).w with 0x1F, queues palette upload from 0x17B1E6 + D0w*4
 * to 0x7A80 (count 0x20) via 0x001660, and frees self (0x0105EC) when
 * (int16_t)(A5+0x4B38) == -1 else re-arms 0x17B1A6. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_17B1A6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d0w = rd16(m, F3_A5 + 0x0878u) & 0x001Fu;    /* 0x17B1A6..0x17B1AA */
    uint32_t src = 0x0017B1E6u + (uint32_t)d0w * 4u;      /* 0x17B1AE..0x17B1B4 */

    Pal_QueueUpload(m, src, 0x7A80u, 0x0020u);            /* 0x17B1B8..0x17B1CC */

    if (rd16(m, F3_A5 + 0x4B38u) != 0xFFFFu) {            /* 0x17B1D0..0x17B1D6 */
        wr32(m, obj, 0x0017B1A6u);                        /* 0x17B19E */
    } else {
        Obj_Free(m, obj);                                 /* 0x17B1D8..0x17B1E0 */
    }

    r->d[0] = setw(r->d[0], d0w);
    r->a[1] = src;
    return 0;
}

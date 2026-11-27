#include "fn_auto.h"
/* ROM 0x1176B4: verified on a capture from the Y27d re-survey states (zone 27, input down): Fill the object table with 0xFF and install its next callback. */

int Fn_1176B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = obj + 0x44u; /* 0x1176B4 */
    r->d[1] = UINT32_MAX;  /* 0x1176BC MOVEQ #-1 */
    for (unsigned i = 0; i < 32u; ++i)
        wr8(m, r->a[1]++, 0xFFu); /* 0x1176BE MOVE.B and DBF */
    r->d[0] |= 0xFFFFu; /* DBF expires with D0.w = -1. */

    wr16(m, obj + 0x3Au, 0u); /* 0x1176C4 */
    wr32(m, obj, 0x1176D0u);  /* 0x1176C8 */
    return 0;
}

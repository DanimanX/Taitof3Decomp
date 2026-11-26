/* ROM 0x122334: delayed boss palette-ramp & perspective intro starter (A0 = object).
 * Counts down +0x06 and re-arms 0x122334 while > 0. Once <= 0, clears +0x22,
 * positions (+0x14, +0x18) from (A5+0x30D2 - 0x01B00000, A5+0x30D6), clears +0x3A,
 * queues palette ramp via 0x00183C (A5+0x30E6 -> 0x12263C, 0x5E80, 0x100, 0x1A),
 * and transitions to 0x122384. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_122334(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x122334 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x122338 bgt */
        wr32(m, obj + 0x00u, 0x00122334u);               /* 0x12232C */
        return 0;
    }

    wr8(m, obj + 0x22u, 0u);                             /* 0x12233A */
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u) - 0x01B00000u); /* 0x12233E..0x122344 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));      /* 0x12234C */
    wr32(m, obj + 0x3Au, 0u);                            /* 0x122352 */

    uint32_t slot = Pal_QueueRamp(m, rd32(m, F3_A5 + 0x30E6u), 0x0012263Cu,
                                  0x00005E80u, 0x00000100u, 0x0000001Au); /* 0x122356..0x122372 */
    r->d[0] = (slot == 0u) ? 0xFFFFFFFFu : slot;
    wr32(m, obj + 0x00u, 0x00122384u);                   /* 0x12237C */
    return 0;
}

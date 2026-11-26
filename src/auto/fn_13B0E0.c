/* ROM 0x13B0E0: dual-cycle palette animation tick (A0 = object).
 * Queues a 15-word palette upload (0x001660) from 0x13B16C[+0x44.b * 4] to
 * dst 0x7204, advances +0x44.w by 0x80 wrapping at +0x44.b >= 15; then queues
 * a 3-word palette upload from 0x13B1FC[+0x48.b * 4] to dst 0x7274, advances
 * +0x48.w by 0x100 wrapping at +0x48.b >= 3. Frees self (0x0105EC) if
 * A5-0x144B bit 4 is set, else re-arms 0x13B0E0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"

int Fn_13B0E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        uint32_t idx = rd8(m, obj + 0x44u);              /* 0x13B0E0 bfextu {0:8} */
        uint32_t src = 0x0013B16Cu + idx * 4u;           /* 0x13B0E6 */
        Pal_QueueUpload(m, src, 0x7204u, 0x000Fu);       /* 0x13B0EE..0x13B0FC */
        wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + 0x0080u)); /* 0x13B106 */
        if ((int8_t)rd8(m, obj + 0x44u) >= 0x0F)         /* 0x13B10C..0x13B112 */
            wr8(m, obj + 0x44u, 0u);                     /* 0x13B116 */
    }

    {
        uint32_t idx = rd8(m, obj + 0x48u);              /* 0x13B11A bfextu {0:8} */
        uint32_t src = 0x0013B1FCu + idx * 4u;           /* 0x13B120 */
        r->d[0] = idx;
        r->a[1] = src;
        Pal_QueueUpload(m, src, 0x7274u, 0x0003u);       /* 0x13B128..0x13B136 */
        wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) + 0x0100u)); /* 0x13B140 */
        if ((int8_t)rd8(m, obj + 0x48u) >= 0x03)         /* 0x13B146..0x13B14C */
            wr8(m, obj + 0x48u, 0u);                     /* 0x13B150 */
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) == 0u) {       /* 0x13B154..0x13B15A */
        wr32(m, obj, 0x0013B0E0u);                       /* 0x13B0D8 */
        return 0;
    }

    Obj_Free(m, obj);                                    /* 0x13B15E..0x13B160 */
    return 0;
}

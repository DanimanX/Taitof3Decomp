/* ROM 0x18E750: boss fan-spread projectile burst tick (A0 = object).
 * Counts down +0x06 and re-arms 0x18E750 while > 0. On zero, copies +0x50/
 * +0x54 from parent (+0x08), spawns (byte +0x42)+1 projectiles of 0x184184
 * across an 0x50 angle arc starting at 0xF4FD0060 + (+0x63), decrements
 * +0x62 by 4: if > 0, branches to 0x18E196 (when A5+0x4B38 == 0) or re-arms
 * 0x18E750 (+0x06=4); else arms 0x18E7B0 (+0x06=0x1E). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_18E750(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x18E750 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                             /* 0x18E754 */
        wr32(m, obj, 0x0018E750u);                      /* 0x18E748 */
        return 0;
    }

    uint32_t parent = rd32(m, obj + 0x08u);             /* 0x18E756 */
    wr16(m, obj + 0x50u, rd16(m, parent + 0x50u));      /* 0x18E75A */
    wr16(m, obj + 0x54u, rd16(m, parent + 0x54u));      /* 0x18E760 */

    uint32_t d1 = 0xF4FD0060u;                          /* 0x18E766 */
    d1 = (d1 & 0xFFFFFF00u) | ((d1 + rd8(m, obj + 0x63u)) & 0xFFu); /* 0x18E76C */
    uint16_t count = rd8(m, obj + 0x42u);               /* 0x18E770: bfextu (0x42,A0){0:8},D7 */
    uint32_t d2 = 0x00000050u;                          /* 0x18E776 */
    if (count != 0u)                                    /* 0x18E778: divu.w D7w,D2 */
        d2 = ((0x50u % count) << 16) | (0x50u / count);

    uint32_t d0 = r->d[0];
    uint16_t loop_cnt = count;
    for (;;) {                                          /* 0x18E77A..0x18E796 */
        d0 = Obj_Spawn(m, obj, 0x00184184u, 9u, 2u, d1);
        d1 = (d1 & 0xFFFFFF00u) | ((d1 + (d2 & 0xFFu)) & 0xFFu);
        if (loop_cnt == 0u)
            break;
        loop_cnt--;
    }

    uint16_t w62 = (uint16_t)(rd16(m, obj + 0x62u) - 4u); /* 0x18E79A */
    wr16(m, obj + 0x62u, w62);
    if ((int16_t)w62 > 0) {                             /* 0x18E79E */
        if (rd16(m, F3_A5 + 0x4B38u) == 0u) {           /* 0x18E736 */
            wr16(m, obj + 0x06u, 0x0002u);              /* 0x18E186 */
            wr32(m, obj, 0x0018E196u);                  /* 0x18E18E */
        } else {
            wr16(m, obj + 0x06u, 0x0004u);              /* 0x18E740 */
            wr32(m, obj, 0x0018E750u);                  /* 0x18E748 */
        }
    } else {
        wr16(m, obj + 0x06u, 0x001Eu);                  /* 0x18E7A0 */
        wr32(m, obj, 0x0018E7B0u);                      /* 0x18E7A8 */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0x0000FFFFu;
    r->a[4] = parent;
    return 0;
}

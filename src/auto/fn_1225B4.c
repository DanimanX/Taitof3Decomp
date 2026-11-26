/* ROM 0x1225B4: boss/background wobble + fade-ramp transition tick (A0 = object).
 * Frees self if parent (+0x08)->+0x59 != 0. Otherwise advances +0x3A by
 * 0x10000, fills 232 line-RAM wobble entries at 0x62A400 + (int16_t)(A5-0x5D56)
 * via 0x176992, decrements +0x06, and re-arms 0x1225B4 while > 0 (unsigned).
 * When +0x06 hits 0, queues palette ramp 0x12263C -> (A5+0x30E6) via 0x00183C,
 * clears +0x22, and transitions to 0x122208. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x176992: fill line RAM entries [d7w..d6w] from sine table 0x027E40. */
static void sub_176992(const f3mem_t *m, f3regs_t *r, uint32_t a0,
                       uint32_t d0, uint32_t d1, uint16_t d2w,
                       uint16_t d6w, uint16_t d7w)
{
    uint32_t a1 = 0x00027E40u;                      /* 0x176992 */
    wr32(m, F3_A5 + 0x4298u, d0);                   /* 0x176998 */
    uint16_t d5w = 0x003Fu;                         /* 0x17699C */
    uint32_t d3 = 0u;                               /* 0x1769A0 */
    a0 += (uint32_t)(int32_t)(int16_t)d7w * 2u;     /* 0x1769A2 */
    for (;;) {
        d3 = (d3 & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x4299u); /* 0x1769A6 */
        int16_t s = (int16_t)rd16(m, a1 + d3 * 2u); /* 0x1769AA */
        int32_t prod = (int32_t)s * (int32_t)(int16_t)d2w; /* 0x1769AE */
        int32_t shifted = (prod < 0)
            ? ~(int32_t)((uint32_t)(~prod) >> 8)
            : (int32_t)((uint32_t)prod >> 8);       /* 0x1769B0 asr.l #8 */
        uint16_t outw = (uint16_t)((uint16_t)(uint32_t)shifted ^ d5w); /* 0x1769B2 */
        d0 = ((uint32_t)shifted & 0xFFFF0000u) | outw;
        wr16(m, a0, outw);                          /* 0x1769B4 */
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d1); /* 0x1769B6 */
        d7w = (uint16_t)(d7w + 1u);                 /* 0x1769BA */
        if ((int16_t)d7w > (int16_t)d6w)            /* 0x1769BC..0x1769BE */
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
}

int Fn_1225B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x1225B4 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                 /* 0x1225B8..0x1225BC */
        Obj_Free(m, obj);                           /* 0x12262E */
        return 0;
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x00010000u); /* 0x1225C0 */

    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x1225D2 */
    uint32_t lram = 0x0062A400u + (uint32_t)(int32_t)off; /* 0x1225CC */
    uint32_t d0 = rd32(m, lram + 0x3Au);            /* 0x1225DE */
    sub_176992(m, r, lram, d0, 0x00040000u, 6u, 0x00E7u, 0u); /* 0x1225F4 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1225FE */
    wr16(m, obj + 0x06u, cnt);
    if (cnt > 0u) {                                 /* 0x122602 bhi -> 0x1225AC */
        wr32(m, obj, 0x001225B4u);
        return 0;
    }

    uint32_t slot = Pal_QueueRamp(m, 0x0012263Cu, rd32(m, F3_A5 + 0x30E6u),
                                  0x00005E80u, 0x00000400u, 0x0000001Au); /* 0x122604..0x122620 */
    r->d[0] = (slot != 0u) ? slot : 0xFFFFFFFFu;
    wr8(m, obj + 0x22u, 0u);                        /* 0x12262A -> 0x1221FC */
    wr32(m, obj, 0x00122208u);                      /* 0x122200 */
    return 0;
}

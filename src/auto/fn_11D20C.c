/* ROM 0x11D20C: boss phase helper tick (A0 = object).
 * Frees self via 0x0105EC if parent (+0x08)->+0x59 != 0. Increments +0x06,
 * iterates bit 0..7 of A5+0x30F2 over table 0x11D26C (entry 0 is bare rts at
 * 0x11D274, entry 1 is 0x11D276 setting A5+0x30EE bit 7 on 0x22-frame cadence),
 * copies A5+0x30F2 to +0x22, advances A5+0x3258 by 0x3333 wrapping >= 0x0C,
 * and re-arms 0x11D20C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CC2BC: frame-rate-compensated modulo timer -> D0w = rem, D0hi = quot. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t d0 = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(d0 / (uint16_t)d1);
    uint16_t rem = (uint16_t)(d0 % (uint16_t)d1);
    *d0_io = ((uint32_t)quot << 16) | rem;
    *d1_io = d1;
}

/* ROM 0x11D276: phase-1 sub-handler; updates D1 when 0x0CC2BC runs. */
static void sub_11D276(const f3mem_t *m, uint32_t obj, uint32_t *d1_io)
{
    if ((rd8(m, obj + 0x22u) & 0x02u) == 0u)            /* 0x11D276 */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x11D280 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x01u) != 0u)        /* 0x11D284: bftst {7:1} */
        return;

    uint32_t tmp_d0 = 0x0022u;
    sub_0CC2BC(m, 0x0022u, &tmp_d0, d1_io);             /* 0x11D28E */
    if ((uint16_t)tmp_d0 != 0u)                         /* 0x11D298 */
        return;

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u)); /* 0x11D2A4 */
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Au) + 1u); /* 0x11D2AA */
    wr16(m, obj + 0x3Au, cnt);
    if ((int16_t)cnt > 2)                               /* 0x11D2AE */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x11D2B8 */
}

int Fn_11D20C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x11D20C */

    if (rd8(m, a1 + 0x59u) != 0u) {                     /* 0x11D210 */
        Obj_Free(m, obj);                               /* 0x11D25E */
        r->a[1] = a1;
        return 0;
    }

    uint8_t flags = rd8(m, F3_A5 + 0x30F2u);
    if ((flags & 0xFCu) != 0u)                          /* entries 2..7 in 0x11D26C are not code */
        return -1;

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u)); /* 0x11D218 */

    /* 0x11D21C..0x11D23E: loop D0=0..7, A1 advances 0x11D26C -> 0x11D28C. */
    if ((flags & 0x02u) != 0u)
        sub_11D276(m, obj, &d1);

    wr8(m, obj + 0x22u, flags);                         /* 0x11D240 */
    wr32(m, F3_A5 + 0x3258u, rd32(m, F3_A5 + 0x3258u) + 0x00003333u); /* 0x11D246 */
    if ((int16_t)rd16(m, F3_A5 + 0x3258u) >= 0x000C)    /* 0x11D24E */
        wr16(m, F3_A5 + 0x3258u, 0u);                   /* 0x11D258 */

    wr32(m, obj, 0x0011D20Cu);                          /* 0x11D25C -> 0x11D204 */
    r->d[0] = 0x00000008u;
    r->d[1] = d1;
    r->a[1] = 0x0011D28Cu;
    return 0;
}

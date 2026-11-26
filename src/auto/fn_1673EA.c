/* ROM 0x1673EA: line-RAM sine-wave fill tick (A0 = object).
 * Calls 0x176992 on 0x62A000 + (A5-0x5D56).w with phase +0x44.l, step
 * 0x0080F000, amplitude 0x60, and range [0..0xE7], then advances +0x44.l by
 * 0x21000 and re-arms 0x1673EA. */
#include "fn_auto.h"

/* ROM 0x176992: fill line RAM entries [d7w..d6w] from sine table 0x027E40. */
static void sub_176992(const f3mem_t *m, f3regs_t *r, uint32_t a0,
                       uint32_t d0, uint32_t d1, uint16_t d2w,
                       uint16_t d6w, uint16_t d7w)
{
    uint32_t a1 = 0x00027E40u;                            /* 0x176992 */
    wr32(m, F3_A5 + 0x4298u, d0);                         /* 0x176998 */
    uint16_t d5w = 0x003Fu;                               /* 0x17699C */
    uint32_t d3 = 0u;                                     /* 0x1769A0 */
    a0 += (uint32_t)(int32_t)(int16_t)d7w * 2u;           /* 0x1769A2 */
    for (;;) {
        d3 = (d3 & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x4299u); /* 0x1769A6 */
        int16_t s = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)d3 * 2u); /* 0x1769AA */
        int32_t prod = (int32_t)s * (int32_t)(int16_t)d2w; /* 0x1769AE */
        int32_t shifted = (prod < 0)
            ? ~(int32_t)((uint32_t)(~prod) >> 8)
            : (int32_t)((uint32_t)prod >> 8);             /* 0x1769B0 asr.l #8 */
        uint16_t outw = (uint16_t)((uint16_t)(uint32_t)shifted ^ d5w); /* 0x1769B2 */
        d0 = ((uint32_t)shifted & 0xFFFF0000u) | outw;
        wr16(m, a0, outw);                                /* 0x1769B4 */
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d1); /* 0x1769B6 */
        d7w = (uint16_t)(d7w + 1u);                       /* 0x1769BA */
        if ((int16_t)d7w > (int16_t)d6w)                  /* 0x1769BC..0x1769BE */
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
}

int Fn_1673EA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, obj + 0x44u);                   /* 0x1673EA */
    uint32_t dst = 0x0062A000u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x1673F2..0x1673F8 */

    sub_176992(m, r, dst, d0, 0x0080F000u, 0x0060u, 0x00E7u, 0u); /* 0x1673FC..0x167418 */

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 0x00021000u); /* 0x167422 */
    wr32(m, obj, 0x001673EAu);                            /* 0x16742A -> 0x1673E2 */
    return 0;
}

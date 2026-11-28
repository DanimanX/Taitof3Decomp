/* ROM 0x1784E6: 16x16 ramp-down of one palette component (A5+0x42B4 palette).
 * Same 16x16 loop and 0x17855A body as 0x1784CA but the entry is decremented
 * by one while it differs from the byte target table 0x1788DE, i.e. 256
 * decrements of one entry per call. */
#include "fn_auto.h"

/* ROM 0x17855A: one decrement step of the palette entry. */
static void sub_17855A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x42B4u;                             /* 0x17855A lea */
    r->a[1] = 0x001788C2u;                                     /* 0x17855E lea (0x362,PC) */
    r->a[2] = 0x001788DEu;                                     /* 0x178564 lea (0x378,PC) */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB4u); /* 0x17856A */
    r->d[5] = (r->d[5] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB8u); /* 0x17856E */
    r->d[4] = (r->d[4] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB6u); /* 0x178572 */
    r->d[3] = (r->d[3] & 0xFFFF0000u)
            | rd16(m, r->a[1] + (uint32_t)(int32_t)(int16_t)r->d[4] * 2u); /* 0x178576 */
    r->d[6] = (r->d[6] & 0xFFFF0000u)
            | (uint16_t)((uint16_t)r->d[6] + (uint16_t)r->d[5]);           /* 0x17857A */
    r->d[6] = (uint32_t)((int32_t)(int16_t)r->d[6] * 8);                   /* 0x17857C muls.w #8 */
    r->a[0] = a0 + (uint32_t)(int32_t)(int16_t)r->d[6];                             /* 0x178580 lea D6w */

    uint32_t addr = r->a[0] + (uint32_t)(int32_t)(int16_t)r->d[3];
    if ((int16_t)r->d[4] >= 6) {                                           /* 0x178584/0x178588 */
        uint8_t v = rd8(m, addr);                                          /* 0x1785A0 */
        uint8_t tgt = rd8(m, r->a[2] + (uint32_t)(int32_t)(int16_t)r->d[3]); /* 0x1785A4 */
        r->d[1] = (r->d[1] & 0xFFFFFF00u) | v;
        if (v == tgt)                                                      /* 0x1785A8 */
            return;
        wr8(m, addr, (uint8_t)(v - 1u));                                   /* 0x1785AC */
    } else {
        uint16_t v = rd16(m, addr);                                        /* 0x17858C */
        uint16_t tgt = rd16(m, r->a[2] + (uint32_t)(int32_t)(int16_t)r->d[3]); /* 0x178590 */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | v;
        if (v == tgt)                                                      /* 0x178594 */
            return;
        wr16(m, addr, (uint16_t)(v - 1u));                                 /* 0x178598 */
    }
}

int Fn_1784E6(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1784E6 move.w #0xf,D7w */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0x000Fu;
    uint16_t d7 = 0x000Fu;
    do {                                                                   /* 0x1784EA */
        uint16_t d0 = 0x000Fu;                                             /* 0x1784F4 */
        do {                                                               /* 0x1784F8 */
            sub_17855A(m, r);                                              /* 0x1784F8 bsr */
            d0--;                                                          /* 0x1784FC dbf */
            r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
        } while (d0 != 0xFFFFu);
        d7--;                                                              /* 0x1784EE dbf */
        r->d[7] = (r->d[7] & 0xFFFF0000u) | d7;
    } while (d7 != 0xFFFFu);
    return 0;                                                              /* 0x1784F2 rts */
}

/* ROM 0x14F3B4: stamp the scroll-window control words.
 * With base = the signed word at A5-0x5D56 (0x4022AA): 0x17668A writes the low
 * nibble 2 into the 0xE8 words at 0x62B200+base (D0 = 2, D7 = 0, D6 = 0xE7),
 * then 0x1766BE masks the 0xE8 words at 0x62B600+base with 0x3FFF (D1 = 0).
 * D6/D7/A0 are saved across the first call only, so A0 returns as the second
 * base, D7 as 0xFFFF and D1 with its low word cleared; D0 keeps the last
 * masked word in its low word. */
#include "fn_auto.h"

/* ROM 0x17668A: D1.w = (A0,D7.w*2), replace its low nibble with D0's, store
 * back, until D7 passes D6 (both words compared signed). D1 keeps its own high
 * word, since 0x17668E bfins D0,D1{28:4} only rewrites bits 3..0. */
static void sub_17668A(const f3mem_t *m, uint32_t a0, uint16_t d0w,
                       uint16_t d6w, uint16_t *d7_io, uint32_t *d1_io)
{
    uint16_t d7 = *d7_io;
    for (;;) {                                                /* 0x17668A */
        uint16_t w = rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u);
        w = (uint16_t)((w & 0xFFF0u) | (d0w & 0x000Fu));      /* 0x17668E */
        wr16(m, a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u, w); /* 0x176692 */
        *d1_io = (*d1_io & 0xFFFF0000u) | w;
        d7 = (uint16_t)(d7 + 1u);                             /* 0x176696 */
        if ((int16_t)d7 > (int16_t)d6w) {                     /* 0x176698 */
            *d7_io = d7;
            return;
        }
    }
}

/* ROM 0x1766BE: D1.w = 0, then D0.w = (A0,D7.w*2) & 0x3FFF | D1.w for
 * D7 = 0xE7..0, i.e. 0xE8 words; D7 is left at 0xFFFF by the dbf. */
static void sub_1766BE(const f3mem_t *m, uint32_t a0, uint32_t *d0_io,
                       uint32_t *d1_io, uint16_t *d7_io)
{
    uint16_t d1w = 0x0000u;                                   /* 0x1766BE */
    uint16_t d7 = 0x00E7u;                                    /* 0x1766C2 */
    for (;;) {                                                /* 0x1766C6 */
        uint16_t w = rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u);
        w = (uint16_t)((w & 0x3FFFu) | d1w);                  /* 0x1766CA/0x1766CE */
        wr16(m, a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u, w); /* 0x1766D0 */
        *d0_io = (*d0_io & 0xFFFF0000u) | w;
        d7 = (uint16_t)(d7 - 1u);                             /* 0x1766D4 dbf */
        if (d7 == 0xFFFFu)
            break;
    }
    *d1_io = (*d1_io & 0xFFFF0000u) | d1w;
    *d7_io = d7;
}

int Fn_14F3B4(const f3mem_t *m, f3regs_t *r)
{
    int16_t base = (int16_t)rd16(m, F3_A5 - 0x5D56u);          /* 0x14F3BE adda.w */

    /* 0x14F3C2..0x14F3D0: push 0, push 0xE7, pop them into D6 then D7. */
    uint16_t d6 = 0x00E7u;
    uint16_t d7 = 0x0000u;
    uint32_t d1 = r->d[1];

    uint32_t a0 = 0x0062B200u + (uint32_t)(int32_t)base;      /* 0x14F3B8 */
    uint32_t d0 = (r->d[0] & 0xFFFF0000u) | 0x0002u;          /* 0x14F3CA move.w #2,D0w */
    sub_17668A(m, a0, (uint16_t)d0, d6, &d7, &d1);            /* 0x14F3D2 */

    /* 0x14F3D8 pops D6/D7/A0; A0 is rebuilt from the scroll word at 0x14F3E2. */
    a0 = 0x0062B600u
         + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x14F3E2 */
    sub_1766BE(m, a0, &d0, &d1, &d7);                         /* 0x14F3E6 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[7] = (r->d[7] & 0xFFFF0000u) | d7;   /* 0x1766D4 dbf: D7 = 0xFFFF */
    r->a[0] = a0;
    return 0;                                                 /* 0x14F3EC */
}

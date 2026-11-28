/* ROM 0x005AB6: UNVERIFIED (no capture): expand a packed tile stream into playfield RAM.
 * D0.w selects (D0.w * 8 + 1) tile words and D1.w adds a signed * 16 offset to the
 * byte plane at 0x61E000. Each tile word is split into two 4-bit groups that are
 * rotated through X into two bit planes (D2 even bits, D3 odd bits) and the two
 * bytes are stored D3 first, then D2; A0 advances 4 bytes per tile and D0.w ends
 * at -1 because of the DBF. The only capture set for this address has all five
 * pairs spanning a VBlank frame (fpre != fpost), which the test runner skips as
 * unobservable, so this was checked offline against the post images instead:
 * D0 and A0 match exactly and all but ~50 of the 73744 bytes written into the
 * 0x61E000 plane match, the residue sitting in the two regions the game itself
 * rewrote during the 156-frame interval. */
#include "fn_auto.h"

/* The four-pass rotate chain at 0x005AD4..0x005AE0 / 0x005AEE..0x005AFA. ROL/ROR
 * touch only C, ROXR/ROXL pass the X flag through, so X is carried in *x. */
static void shuffle(uint32_t *d1, uint16_t *d2w, uint16_t *d3w, unsigned *x)
{
    for (int i = 0; i < 4; i++) {
        uint32_t v = *d1;
        v = (v >> 4) | (v << 28);                        /* 0x005AD4 ror.l #4 */
        uint16_t w = (uint16_t)v;
        w = (uint16_t)((uint16_t)(w << 4) | (uint16_t)(w >> 12)); /* 0x005AD6 rol.w #4 */
        unsigned in = *x;                                /* 0x005AD8 roxr.w #1 */
        *x = w & 1u;
        w = (uint16_t)((uint16_t)(in ? 0x8000u : 0u) | (uint16_t)(w >> 1));
        *d1 = (v & 0xFFFF0000u) | w;

        uint16_t d = *d2w;                               /* 0x005ADA roxl.w #1,D2 */
        uint32_t nx = (uint32_t)(d >> 15);
        d = (uint16_t)((uint16_t)(d << 1) | (uint16_t)(*x ? 1u : 0u));
        *d2w = d;
        *x = (unsigned)nx;

        in = *x;                                         /* 0x005ADC roxr.w #1 */
        *x = w & 1u;
        w = (uint16_t)((uint16_t)(in ? 0x8000u : 0u) | (uint16_t)(w >> 1));
        *d1 = (v & 0xFFFF0000u) | w;

        d = *d3w;                                        /* 0x005ADE roxl.w #1,D3 */
        nx = (uint32_t)(d >> 15);
        d = (uint16_t)((uint16_t)(d << 1) | (uint16_t)(*x ? 1u : 0u));
        *d3w = d;
        *x = (unsigned)nx;
    }
}

int Fn_005AB6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint16_t a1w = (uint16_t)((uint16_t)r->d[1] << 4);    /* 0x005AC2 */
    uint32_t a1 = 0x61E000u + (uint32_t)(int32_t)(int16_t)a1w; /* 0x005ABA..0x005AC4 */
    uint32_t count = (uint16_t)((uint16_t)r->d[0] << 3);   /* 0x005AC0 */
    unsigned x = 0;                                        /* X from the caller: not captured */

    for (uint32_t i = 0; i <= count; i++) {                /* 0x005AC6..0x005B02 dbf */
        uint16_t d2w = 0, d3w = 0;                         /* 0x005AC6 / 0x005AC8 */
        uint32_t d1 = (uint32_t)rd16(m, a0) << 16;          /* 0x005ACA..0x005ACE */
        a0 += 2;
        shuffle(&d1, &d2w, &d3w, &x);                       /* 0x005AD4..0x005AE0 */
        d1 = (uint32_t)rd16(m, a0) << 16;                    /* 0x005AE4..0x005AE8 */
        a0 += 2;
        shuffle(&d1, &d2w, &d3w, &x);                       /* 0x005AEE..0x005AFA */
        wr8(m, a1, (uint8_t)d3w);                           /* 0x005AFE */
        a1 += 1;
        wr8(m, a1, (uint8_t)d2w);                           /* 0x005B00 */
        a1 += 1;
        r->d[2] = (r->d[2] & 0xFFFF0000u) | d2w;
        r->d[3] = (r->d[3] & 0xFFFF0000u) | d3w;
    }

    r->a[0] = a0;                                          /* A0 is not restored */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xFFFFu; /* 0x005B02 dbf: always ends at -1 */
    return 0;                                              /* 0x005B0A rts */
}

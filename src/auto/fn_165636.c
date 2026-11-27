/* ROM 0x165636: fill two 0xE8-entry gfx ramps (words at 0x62A200+d and bytes
 * at 0x628200+d, d = sign16[0x4022AA]) with 0x176940's pseudo-random plasma,
 * then initialise the screen-layer registers: 0x40C20C/0x40C210 = 0/0x100000
 * (also 0x402260/0x402264), 0x40C22C/0x40C230 = 0x50000/0, byte 0x40C285 = 4,
 * word 0x40C25E = 0x5BF and the two 0x40C26C = 0x1665A2 / 0x40C270 = 0x166612
 * (D0/D1). Only A0 is saved (0x165636/0x1656D6), so D0-D7 come back as the
 * calls left them; A1 ends 4 past 0x402260. */
#include "fn_auto.h"

/* divs.w: 32-bit dividend in Dn, 16-bit signed divisor -> quotient in Dn's low
 * word, remainder in its high word. On overflow (or a zero divisor, which would
 * trap on the 68k) Dn is left unchanged. */
static uint32_t Fn165636_DivsW(uint32_t dn, uint16_t divisor)
{
    int64_t d = (int64_t)(int32_t)dn;
    int64_t v = (int64_t)(int16_t)divisor;

    if (v == 0) return dn;
    int64_t q = d / v;
    if (q < -32768 || q > 32767) return dn;          /* V: register preserved */
    int64_t r = d % v;
    return (((uint32_t)r & 0xFFFFu) << 16) | ((uint32_t)q & 0xFFFFu);
}

/* ROM 0x00176940: A0 = word array, A1 = byte array, D7..D6 inclusive index.
 * Each step: D4 = Rng*oldD4 + D5 (swapped), the angle from -0xA000/(0xA0+D4w)
 * drives both the byte (D0) and the eor'd ramp word (D2). */
static void Fn165636_Plasma(const f3mem_t *m, uint32_t a0, uint32_t a1,
                            int16_t d6, int16_t d7, uint32_t *d4p,
                            uint32_t *d5p, uint32_t *d0p, uint32_t *d1p,
                            uint32_t *d2p)
{
    uint32_t d4 = *d4p, d5 = *d5p;
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p;

    d4 = (d4 << 1) + 0x8000u;                        /* 0x176940 add.l D4,D4 ; addi.l #0x8000 */
    d4 = (d4 << 16) | (d4 >> 16);                    /* 0x176948 swap D4 (dead: reloaded below) */
    for (;;) {
        d0 = Rng_NextMem(m);                         /* 0x17694A jsr 0x1D8A */
        d0 = (uint32_t)((int32_t)(int16_t)(uint16_t)d4 *
                        (int32_t)(int16_t)(uint16_t)d0);   /* 0x176950 muls.w D4w,D0 */
        d0 += d5;                                    /* 0x176952 add.l D5,D0 */
        d4 = d0;                                     /* 0x176954 move.l D0,D4 */
        d4 = (d4 << 16) | (d4 >> 16);                /* 0x176956 swap D4 */
        d0 = 0xFFFF6000u;                            /* 0x176958 move.l #-0xa000,D0 */
        d1 = 0x000000A0u;                            /* 0x17695E move.l #0xa0,D1 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((d1 & 0xFFFFu) + (d4 & 0xFFFFu)); /* 0x176964 add.w D4w,D1w */
        d0 = Fn165636_DivsW(d0, (uint16_t)d1);       /* 0x176966 divs.w D1w,D0 */
        d2 = (d2 & 0xFFFF0000u) | (d0 & 0xFFFFu);    /* 0x176968 move.w D0w,D2w */
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((d0 & 0xFFFFu) + 0x100u); /* 0x17696A addi.w #0x100,D0w */
        d2 = (d2 & 0xFFFF0000u) | 0x0024u;           /* 0x17696E move.w #0x24,D2w */
        d2 = (d2 & 0xFFFF0000u) |
             (uint16_t)((d2 & 0xFFFFu) - rd16(m, F3_A5 + 0x42A0u)); /* 0x176972 sub.w (0x42a0,A5),D2w */
        d2 = (uint32_t)((int32_t)(int16_t)(uint16_t)d2 *
                        (int32_t)(int16_t)(uint16_t)d0);   /* 0x176976 muls.w D0w,D2 */
        d2 = d2 << 6;                                /* 0x176978 asl.l #0x6,D2 */
        d2 = Fn165636_DivsW(d2, 0x0032u);            /* 0x17697A divs.w #0x32,D2 */
        d2 = (d2 & 0xFFFF0000u) | (uint16_t)((d2 & 0xFFFFu) ^ 0x003Fu); /* 0x17697E eori.w #0x3f */
        wr16(m, a0 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint16_t)d2); /* 0x176982 */
        wr8(m, a1 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint8_t)d0);   /* 0x176986 */
        d7++;                                        /* 0x17698A addq.w #1,D7w */
        if ((int16_t)d7 > d6) break;                 /* 0x17698C cmp.w D6w,D7w ; ble */
    }
    *d4p = d4; *d5p = d5; *d0p = d0; *d1p = d1; *d2p = d2;
}

int Fn_165636(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];                                  /* saved at 0x165636 */
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2];
    uint32_t d4 = 0x00060000u;                               /* 0x16565C move.l #0x60000,D4 */
    uint32_t d5 = 0x000C0000u;                               /* 0x165662 pop D5 (pushed 0xc0000) */
    int16_t  d6 = 0x00E7;                                    /* 0x165664 pop D6.w */
    int16_t  d7 = 0x0000;                                    /* 0x165666 pop D7.w */

    /* 0x16563A..0x165668: A0 = 0x62A200+d, A1 = 0x628200+d, D4/D5/D6/D7 */
    Fn165636_Plasma(m, 0x62A200u + off, 0x628200u + off, d6, d7,
                    &d4, &d5, &d0, &d1, &d2);            /* 0x165668 jsr 0x176940 */

    /* 0x16566E..0x165686: 0x16AB2C with A0 = A5+0x420C, A1 = A5-0x5DA0,
     * D0 = 0, D1 = 0x100000 -> (A0)+,(A0),(A1)+,(A1) */
    wr32(m, F3_A5 + 0x420Cu, 0u);
    wr32(m, F3_A5 + 0x4210u, 0x00100000u);
    wr32(m, F3_A5 - 0x5DA0u, 0u);
    wr32(m, F3_A5 - 0x5D9Cu, 0x00100000u);
    d0 = 0u;
    d1 = 0x00100000u;

    /* 0x16568C..0x1656A0: 0x16AB26 with A0 = A5+0x422C, D0 = 0x50000, D1 = 0 */
    wr32(m, F3_A5 + 0x422Cu, 0x00050000u);
    wr32(m, F3_A5 + 0x4230u, 0u);
    d0 = 0x00050000u;
    d1 = 0u;

    /* 0x1656A6..0x1656AE: 0x16AB58 -> byte at A5+0x4285 = 4 */
    wr8(m, F3_A5 + 0x4285u, 0x04u);
    d0 = (d0 & 0xFFFFFF00u) | 0x04u;                     /* move.b #0x4,D0b */

    wr16(m, F3_A5 + 0x425Eu, 0x05BFu);                   /* 0x1656B4 move.w #0x5bf,(0x425e,A5) */

    /* 0x1656BA..0x1656D0: 0x16AB5C with A0 = 0x40C26C, D0 = 0x1665A2,
     * D1 = 0x166612 -> (A0)+ = D0, (A0)+ = D1 */
    d0 = 0x001665A2u;
    d1 = 0x00166612u;
    wr32(m, 0x0040C26Cu, d0);
    wr32(m, 0x0040C270u, d1);

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[4] = d4;
    r->d[5] = d5;
    r->d[6] = (r->d[6] & 0xFFFF0000u) | (uint16_t)d6;
    r->d[7] = (r->d[7] & 0xFFFF0000u) | (uint16_t)d7;
    r->a[1] = F3_A5 - 0x5DA0u + 4u;                      /* 0x16AB30 (A1)+ */
    wr32(m, obj, 0x001656E2u);                           /* 0x1656DA move.l #0x1656e2,(A0) */
    return 0;                                            /* 0x1656E0 rts */
}

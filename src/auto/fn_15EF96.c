/* ROM 0x15EF96: build the stage's line/plasma ramps and reset the character
 * and line RAM. Puts nibble 9 in the low 4 bits of the 0xE8 words at
 * 0x62B200+d (0x17668A), byte A5+0x4285 = 6, the 0x40C26C pair = 0x15FD1C /
 * 0x15FEEC, the 0x40C22C pair = 0x10000 / 0x10000, copies 0x402258/0x40225C
 * into 0x40C20C/0x40C210 and 0x402260/0x402264 (0x16AB2C), runs the plasma
 * generator 0x176832 over 0x62A200+d/0x628200+d, sets word 0x40C25E = 0x12BF,
 * then clears 0x611000 (0x400 longs), 0x62A200 (0x100 words) and the four
 * A5-0x5DA0/A5-0x5D78 counters (0x5918). A0 ends at 0x62A200+d (0x176832 does
 * not save it), A1/A5-related words keep the 0x16AB2C leftovers, and D0/D1 are
 * the values read from 0x402258/0x40225C. */
#include "fn_auto.h"

/* divs.w: 32-bit dividend, 16-bit signed divisor -> quotient in the low word,
 * remainder in the high word; the register is preserved on overflow (and on a
 * zero divisor, which would trap on the 68k). */
static uint32_t Fn15EF96_DivsW(uint32_t dn, uint16_t divisor)
{
    int64_t d = (int64_t)(int32_t)dn;
    int64_t v = (int64_t)(int16_t)divisor;

    if (v == 0) return dn;
    int64_t q = d / v;
    if (q < -32768 || q > 32767) return dn;
    int64_t r = d % v;
    return (((uint32_t)r & 0xFFFFu) << 16) | ((uint32_t)q & 0xFFFFu);
}

/* ROM 0x0017668A: bits 3..0 of words D7..D6 at A0 := the low 4 bits of D0.w. */
static void Fn15EF96_Nibbles(const f3mem_t *m, uint32_t a0, uint16_t val,
                             int16_t start, int16_t end, uint32_t *d1)
{
    int16_t i = start;
    do {                                              /* bottom-tested: 0x176696 addq ; cmp.w ; ble */
        uint32_t ea = a0 + (uint32_t)(uint16_t)((uint16_t)i * 2u);
        uint16_t w = rd16(m, ea);                     /* 0x17668A */
        w = (uint16_t)((w & 0xFFF0u) | (val & 0x000Fu)); /* 0x17668E bfins D0,D1{0x1c:0x4} */
        wr16(m, ea, w);                               /* 0x176692 */
        *d1 = (*d1 & 0xFFFF0000u) | w;
        i++;
    } while (i <= end);
}

/* ROM 0x00176832: the plasma/gradient generator. A0 = word array, A1 = byte
 * array, D7 = first index, D6 = last index, A5+0x4298 = phase (long, advanced
 * by D5 += D3 each step), A5+0x42A0 = a bias word, D4 = +1 or -1 depending on
 * whether the walk runs upwards or downwards. */
static void Fn15EF96_Plasma(const f3mem_t *m, uint32_t a0, uint32_t a1,
                            int16_t first, int16_t last, uint32_t *phase,
                            int32_t d5, int32_t d3, uint16_t bias)
{
    int16_t d7 = first;
    int16_t d4 = (first <= last) ? 1 : -1;           /* 0x176866/0x17686A/0x176870 */
    uint32_t d0, d1, d2;

    for (;;) {                                       /* 0x176872 */
        d2 = (*phase) & 0xFFFFu;                     /* 0x176872 move.w (0x4298,A5),D2w */
        d0 = 0xFFFF6000u;                            /* 0x176876 move.l #-0xa000,D0 */
        d1 = 0x000000A0u;                            /* 0x17687C move.l #0xa0,D1 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((d1 & 0xFFFFu) + d2); /* 0x176882 add.w D2w,D1w */
        d0 = Fn15EF96_DivsW(d0, (uint16_t)d1);       /* 0x176884 divs.w D1w,D0 */
        d2 = (d2 & 0xFFFF0000u) | (d0 & 0xFFFFu);    /* 0x176886 move.w D0w,D2w */
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((d0 & 0xFFFFu) + 0x100u); /* 0x176888 addi.w #0x100 */
        d2 = (d2 & 0xFFFF0000u) | 0x0024u;           /* 0x17688C move.w #0x24,D2w */
        d2 = (d2 & 0xFFFF0000u) | (uint16_t)((d2 & 0xFFFFu) - bias); /* 0x176890 sub.w (0x42a0,A5) */
        d2 = (uint32_t)((int32_t)(int16_t)(uint16_t)d2 *
                        (int32_t)(int16_t)(uint16_t)d0);   /* 0x176894 muls.w D0w,D2 */
        d2 = d2 << 6;                                /* 0x176896 asl.l #0x6,D2 */
        d2 = Fn15EF96_DivsW(d2, 0x0032u);            /* 0x176898 divs.w #0x32,D2 */
        d2 = (d2 & 0xFFFF0000u) | (uint16_t)((d2 & 0xFFFFu) ^ 0x003Fu); /* 0x17689C eori.w #0x3f */
        wr16(m, a0 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint16_t)d2); /* 0x1768A0 */
        wr8(m, a1 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint8_t)d0);   /* 0x1768A4 */

        d5 += d3;                                    /* 0x1768A8 add.l D3,D5 */
        *phase += (uint32_t)d5;                      /* 0x1768AA add.l D5,(0x4298,A5) */
        d7 = (int16_t)(uint16_t)(d7 + d4);           /* 0x1768AE add.w D4w,D7w */
        if (d4 >= 0) {                               /* 0x1768B0 tst.w D4w ; bpl 0x1768be */
            if (d7 > last) break;                    /* 0x1768BE cmp.w D6w,D7w ; ble */
        } else {
            if (last <= d7) break;                   /* 0x1768B6 cmp.w D7w,D6w ; ble */
        }
    }
}

int Fn_15EF96(const f3mem_t *m, f3regs_t *r)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t d0, d1 = r->d[1];
    uint32_t phase = 0;

    /* 0x15EF9A..0x15EFB4: nibble 9 in words 0..0xE7 of 0x62B200+d (A0/D6/D7
     * saved and restored, so only D1 is scratched and it is reloaded below) */
    Fn15EF96_Nibbles(m, 0x62B200u + off, 9u, 0, 0xE7, &d1);

    wr8(m, F3_A5 + 0x4285u, 0x06u);                  /* 0x15EFBE/0x15EFC6 -> 0x16AB58 */
    wr32(m, 0x0040C26Cu, 0x0015FD1Cu);               /* 0x15EFCC..0x15EFE2 -> 0x16AB5C */
    wr32(m, 0x0040C270u, 0x0015FEECu);
    wr32(m, F3_A5 + 0x422Cu, 0x00010000u);           /* 0x15EFE8..0x15EFFC -> 0x16AB26 */
    wr32(m, F3_A5 + 0x4230u, 0x00010000u);

    /* 0x15F002..0x15F016: 0x16AB2C with A0 = A5+0x420C, A1 = A5-0x5DA0 and
     * D0/D1 popped from A5-0x5DA8 / A5-0x5DA4 */
    d0 = rd32(m, F3_A5 - 0x5DA8u);
    d1 = rd32(m, F3_A5 - 0x5DA4u);
    wr32(m, F3_A5 + 0x420Cu, d0);
    wr32(m, F3_A5 + 0x4210u, d1);
    wr32(m, F3_A5 - 0x5DA0u, d0);
    wr32(m, F3_A5 - 0x5D9Cu, d1);

    /* 0x15F01C..0x15F046: 0x176832 (A0 = 0x62A200, A1 = 0x628200, D7 = 0,
     * D6 = 0xE7, A5+0x4298 = 0, D5 = 0, D3 = 0, A5+0x42A0 = 1) */
    wr32(m, F3_A5 + 0x4298u, 0u);                    /* 0x176852 move.l (0x14,A6),(0x4298,A5) */
    wr16(m, F3_A5 + 0x42A0u, 1u);                    /* 0x176860 move.w (0x20,A6),(0x42a0,A5) */
    Fn15EF96_Plasma(m, 0x62A200u + off, 0x628200u + off, 0, 0xE7, &phase, 0, 0, 1u);

    wr16(m, F3_A5 + 0x425Eu, 0x12BFu);               /* 0x15F050 move.w #0x12bf,(0x425e,A5) */

    /* ROM 0x00005918: clear 0x611000 (0x400 longs), 0x62A200 (0x100 words)
     * and the four A5-0x5DA0/A5-0x5D78 counters. Restores D0/D1/A0/A1. */
    for (uint32_t i = 0; i < 0x400u; i++)            /* 0x005928 move.l D1,(A0)+ ; dbf */
        wr32(m, 0x00611000u + i * 4u, 0u);
    for (uint32_t i = 0; i < 0x100u; i++)            /* 0x005938 move.w D1w,(A0)+ ; dbf */
        wr16(m, 0x0062A200u + i * 2u, 0u);
    wr32(m, F3_A5 - 0x5DA0u, 0u);                    /* 0x005940 */
    wr16(m, F3_A5 - 0x5D9Cu, 0u);                    /* 0x005944 */
    wr32(m, F3_A5 - 0x5D78u, 0u);                    /* 0x005948 */
    wr16(m, F3_A5 - 0x5D74u, 0u);                    /* 0x00594C */

    r->d[0] = d0;                                    /* restored by both callees */
    r->d[1] = d1;
    r->a[0] = 0x62A200u + off;                       /* 0x176832 leaves A0 live */
    r->a[1] = F3_A5 - 0x5DA0u + 4u;                  /* 0x16AB30 (A1)+ */
    return 0;                                        /* 0x15F05C rts */
}

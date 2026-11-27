/* ROM 0x158758: write a 4-bit nibble into the top nibble-pair of 0xE8 words.
 * Two calls to 0x17668A: (0x62B600 + sign16[0x4022AA], nibble 3) and
 * (0x62B400 + same offset, nibble 4), each filling words 0..0xE7. 0x17668A
 * keeps the loop index in D7 and the last word in D1; A0/D6/D7 are restored by
 * the movem at the end of each block, so only D1 is left changed at rts. */
#include "fn_auto.h"

/* ROM 0x17668A: D7..D6 (inclusive) of the word array at A0: bits 3..0 of each
 * word := the low 4 bits of D0.w. D1 returns the last word written. */
static void Fn158758_Fill(const f3mem_t *m, uint32_t a0, uint16_t val,
                          int16_t start, int16_t end, uint32_t *d1)
{
    int16_t i = start;
    do {                                              /* bottom-tested: 0x176696 addq ; cmp.w ; ble */
        uint32_t ea = a0 + (uint32_t)(uint16_t)((uint16_t)i * 2u);
        uint16_t w = rd16(m, ea);                     /* 0x17668A move.w (0x0,A0,D7w*0x2),D1w */
        w = (uint16_t)((w & 0xFFF0u) | (val & 0x000Fu));   /* 0x17668E bfins D0,D1{0x1c:0x4} */
        wr16(m, ea, w);                               /* 0x176692 move.w D1w,(0x0,A0,D7w*0x2) */
        *d1 = (*d1 & 0xFFFF0000u) | w;
        i++;
    } while (i <= end);
}

int Fn_158758(const f3mem_t *m, f3regs_t *r)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, 0x4022AAu); /* (-0x5d56,A5) */
    uint32_t d1 = r->d[1];

    /* 0x158766..0x158776: push 0 and 0xe7, D0.w = 3, pop D6 = 0xe7 / D7 = 0 */
    Fn158758_Fill(m, 0x62B600u + off, 3u, 0, 0xE7, &d1);   /* jsr 0x17668A */
    /* 0x15878E..0x15879E: the same with base 0x62b400 and D0.w = 4 */
    Fn158758_Fill(m, 0x62B400u + off, 4u, 0, 0xE7, &d1);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x0004u;      /* 0x158796 move.w #0x4,D0w */
    r->d[1] = d1;                                     /* not restored by the movem */
    /* A0, D6 and D7 are restored from the stack in both blocks */
    return 0;                                         /* 0x1587A8 rts */
}

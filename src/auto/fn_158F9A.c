/* ROM 0x158F9A: build the stage-2 "sea/plasma" line-RAM bands. Puts nibble 4
 * in the low bits of words 0..0xE7 at 0x62B400+d (0x17668A), runs the plasma
 * generator (0x176832) four times over 0x62A400+d / 0x628400+d and once over
 * 0x62A600+d / 0x628600+d, puts nibble 3 in the words at 0x62B600+d, clears
 * A5+0x4287, stores the pair 0x6000 / 0 into A5+0x423C/0x4240 (0x16AB26), and
 * blits eight compressed tile blocks into 0x613000..0x613460.
 *
 * d = (A5-0x5D56) (the vertical scroll); the generator also leaves A5+0x4298
 * at the last walk's accumulated phase and A5+0x42A0 at its bias. At rts
 * D0 = 0x6000, D1 = 0 (from 0x16AB26) and A0 = A5+0x4240 (0x16AB26 post-
 * increments A0, and 0x0053B2 preserves it); A1 and D6/D7 keep their entry
 * values. */
#include "fn_auto.h"
#include "../video/sprite.h"

/* divs.w: 32-bit dividend, 16-bit signed divisor -> quotient in the low word,
 * remainder in the high word; the register is preserved on overflow. */
static uint32_t Fn158F9A_DivsW(uint32_t dn, uint16_t divisor)
{
    int64_t d = (int64_t)(int32_t)dn;
    int64_t v = (int64_t)(int16_t)divisor;

    if (v == 0) return dn;
    int64_t q = d / v;
    if (q < -32768 || q > 32767) return dn;
    int64_t r = d % v;
    return (((uint32_t)r & 0xFFFFu) << 16) | ((uint32_t)q & 0xFFFFu);
}

/* ROM 0x0017668A: bits 3..0 of words d7..d6 at A0 := the low 4 bits of D0.w */
static void fn158f9a_nibbles(const f3mem_t *m, uint32_t a0, uint16_t val,
                             int16_t start, int16_t end)
{
    int16_t i = start;
    do {                                              /* 0x176696 addq ; cmp.w ; ble */
        uint32_t ea = a0 + (uint32_t)(uint16_t)((uint16_t)i * 2u);
        uint16_t w = rd16(m, ea);                     /* 0x17668A */
        w = (uint16_t)((w & 0xFFF0u) | (val & 0x000Fu)); /* 0x17668E bfins D0,D1{0x1c:0x4} */
        wr16(m, ea, w);                               /* 0x176692 */
        i = (int16_t)(uint16_t)(i + 1);
    } while (i <= end);
}

/* ROM 0x00176832: plasma/gradient walk. A0 = word array, A1 = byte array (one
 * byte per two, D7.w*2 apart), D7 = first index, D6 = last index,
 * A5+0x4298 = phase (advanced by D5 += D3 each step), A5+0x42A0 = bias.
 * Both A5 words are left at their last values, A0/A1 are not restored. */
static void fn158f9a_plasma(const f3mem_t *m, uint32_t a0base, uint32_t a1base,
                            int16_t first, int16_t last, uint32_t phase0,
                            int32_t d5, int32_t d3, uint16_t bias)
{
    uint32_t soff = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a0 = a0base + soff;
    uint32_t a1 = a1base + soff;
    uint32_t phase = phase0;
    int16_t d7 = first;
    int16_t d4 = (first <= last) ? 1 : -1;             /* 0x176866/0x17686A/0x176870 */

    wr32(m, F3_A5 + 0x4298u, phase);                   /* 0x176852 */
    wr16(m, F3_A5 + 0x42A0u, bias);                    /* 0x176860 */

    for (;;) {                                         /* 0x176872 */
        uint32_t d2 = (phase >> 16) & 0xFFFFu;         /* move.w (0x4298,A5): HIGH word */
        uint32_t d0 = 0xFFFF6000u;                     /* move.l #-0xa000,D0 */
        uint32_t d1 = 0x000000A0u;                     /* move.l #0xa0,D1 */
        d1 = (uint16_t)((uint16_t)d1 + (uint16_t)d2);  /* 0x176882 add.w D2w,D1w */
        d0 = Fn158F9A_DivsW(d0, (uint16_t)d1);         /* 0x176884 divs.w D1w,D0 */
        d2 = d0 & 0xFFFFu;                             /* 0x176886 move.w D0w,D2w */
        d0 = (uint16_t)((d0 & 0xFFFFu) + 0x100u);      /* 0x176888 addi.w #0x100 */
        uint32_t m2 = (uint16_t)(0x0024u - bias);      /* 0x17688C/0x176890 */
        m2 = (uint32_t)((int32_t)(int16_t)(uint16_t)m2 *
                        (int32_t)(int16_t)(uint16_t)d0); /* 0x176894 muls.w D0w,D2 */
        m2 = m2 << 6;                                  /* 0x176896 asl.l #0x6,D2 */
        m2 = Fn158F9A_DivsW(m2, 0x0032u);              /* 0x176898 divs.w #0x32,D2 */
        m2 = (uint16_t)((uint16_t)m2 ^ 0x003Fu);       /* 0x17689C eori.w #0x3f */
        wr16(m, a0 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint16_t)m2); /* 0x1768A0 */
        wr8(m, a1 + (uint32_t)(uint16_t)((uint16_t)d7 * 2u), (uint8_t)d0);   /* 0x1768A4 */

        d5 += d3;                                      /* 0x1768A8 add.l D3,D5 */
        phase += (uint32_t)d5;                         /* 0x1768AA add.l D5,(0x4298,A5) */
        wr32(m, F3_A5 + 0x4298u, phase);
        d7 = (int16_t)(uint16_t)(d7 + d4);             /* 0x1768AE add.w D4w,D7w */
        if (d4 >= 0) {                                 /* 0x1768B0 tst.w D4w ; bpl */
            if (d7 > last) break;                      /* 0x1768BE cmp.w D6w,D7w ; ble loops */
        } else {
            if (last > d7) break;                      /* 0x1768B6 cmp.w D7w,D6w ; ble loops */
        }
    }
}

/* ROM 0x0016AB26: store D0/D1 as a long pair at A0 and leave A0 past it */
static void fn158f9a_store_pair(const f3mem_t *m, uint32_t a0,
                                uint32_t d0, uint32_t d1)
{
    wr32(m, a0, d0);                                   /* move.l D0,(A0)+ */
    wr32(m, a0 + 4u, d1);
}

int Fn_158F9A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t soff = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    /* 0x158F9A..0x158FBE: nibble 4 over 0x62B400+d, words 0..0xE7 */
    fn158f9a_nibbles(m, 0x0062B400u + soff, 4u, 0, 0x00E7);

    /* 0x158FC2..0x159054: three plasma walks over 0x62A400+d / 0x628400+d */
    fn158f9a_plasma(m, 0x0062A400u, 0x00628400u, 0x005F, 0x0000, 0u, 0, 0x0800, 2u);
    fn158f9a_plasma(m, 0x0062A400u, 0x00628400u, 0x0060, 0x0087, 0u, 0, 0x0000, 2u);
    fn158f9a_plasma(m, 0x0062A400u, 0x00628400u, 0x0087, 0x00E7, 0u, 0, 0x0800, 2u);
    /* 0x15905E..0x159088: one more over 0x62A600+d / 0x628600+d, bias 3 */
    fn158f9a_plasma(m, 0x0062A600u, 0x00628600u, 0x0000, 0x00E7, 0u, 0, 0x0000, 3u);

    /* 0x159092..0x1590B6: nibble 3 over 0x62B600+d, words 0..0xE7 */
    fn158f9a_nibbles(m, 0x0062B600u + soff, 3u, 0, 0x00E7);

    wr8(m, F3_A5 + 0x4287u, 0u);                       /* 0x1590BA -> 0x16AB58 */
    /* 0x1590C8..0x1590DC: 0x16AB26 with D0 = 0x6000, D1 = 0 at A5+0x423C */
    fn158f9a_store_pair(m, F3_A5 + 0x423Cu, 0x00006000u, 0u);

    /* 0x1590E2..0x1591AE: eight compressed tile blocks into 0x613000.. */
    Gfx_DrawCompressedTilemap(m, 0x000766B4u, 0x00613000u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x0007677Cu, 0x00613400u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x0007677Cu, 0x00613020u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x000766B4u, 0x00613420u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x000766B4u, 0x00613040u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x0007677Cu, 0x00613440u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x0007677Cu, 0x00613060u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x000766B4u, 0x00613460u, 0u);

    r->d[0] = 0x00006000u;                             /* 0x16AB26 leftovers */
    r->d[1] = 0u;
    r->a[0] = F3_A5 + 0x4240u;                         /* A0 past the stored pair */
    return 0;                                          /* 0x1591B2 rts */
}

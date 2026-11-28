/* ROM 0x137584: (re)initialise the four gfx RAM strips the 0x137584 state owns
 * and the A4+8..+0x1C motion block. A5+0x34FA = 0 only arms the A5+0x34FC flag;
 * the flipped layout (A5-0x7EC2 != 0) fills the strips backwards. */
#include "ea_auto.h"

int Ea_137584(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a4 = r->a[4];
    const uint32_t a5 = r->a[5];
    const uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1];
    uint32_t a0 = r->a[0];

    if (rd16(m, a5 + 0x34FAu) == 0u) {               /* 0x137584 bne 0x137594 */
        wr16(m, a5 + 0x34FCu, 0xFFFFu);              /* 0x13758C */
        r->a[7] = sp + 4u;                           /* 0x137592 rts */
        return 0;
    }
    wr16(m, a5 - 0x60FAu, 0xFFFFu);                  /* 0x137594 */
    /* 0x13759A: strip 1 = 0x300F words (0x627230 forwards / 0x6273D0 back) */
    if (rd16(m, a5 - 0x7EC2u) == 0u) {               /* 0x13759E bne 0x1375BA */
        a0 = 0x627230u;                              /* 0x1375A2 */
        d0 = 0u;                                     /* 0x1375A8 moveq #0,d0 */
        d1 = 0x300Fu;                                /* 0x1375AC */
        do {
            wr16(m, a0, (uint16_t)d1);               /* 0x1375B0 move.w d1,(a0)+ */
            a0 += 2u;
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x1375B2 dbra d0 */
    } else {
        a0 = 0x6273D0u;                              /* 0x1375BA */
        d0 = 0u;
        d1 = 0x300Fu;
        do {
            a0 -= 2u;                                /* 0x1375C8 move.w d1,-(a0) */
            wr16(m, a0, (uint16_t)d1);
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x1375CA dbra d0 */
    }
    /* 0x1375CE: strip 2 = 0x300 words */
    if (rd16(m, a5 - 0x7EC2u) == 0u) {               /* 0x1375D2 bne 0x1375EE */
        a0 = 0x627430u;                              /* 0x1375D6 */
        d0 = 0u;
        d1 = 0x0300u;                                /* 0x1375E0 */
        do {
            wr16(m, a0, (uint16_t)d1);               /* 0x1375E4 */
            a0 += 2u;
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x1375E6 dbra */
    } else {
        a0 = 0x6275D0u;                              /* 0x1375EE */
        d0 = 0u;
        d1 = 0x0300u;                                /* 0x1375F8 */
        do {
            a0 -= 2u;                                /* 0x1375FC */
            wr16(m, a0, (uint16_t)d1);
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x1375FE dbra */
    }
    /* 0x137602: strip 3 = 0x1BAE words */
    if (rd16(m, a5 - 0x7EC2u) == 0u) {               /* 0x137606 bne 0x137622 */
        a0 = 0x627630u;                              /* 0x13760A */
        d0 = 0u;
        d1 = 0x1BAEu;                                /* 0x137614 */
        do {
            wr16(m, a0, (uint16_t)d1);               /* 0x137618 */
            a0 += 2u;
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x13761A dbra */
    } else {
        a0 = 0x6277D0u;                              /* 0x137622 */
        d0 = 0u;
        d1 = 0x1BAEu;                                /* 0x13762C */
        do {
            a0 -= 2u;                                /* 0x137630 */
            wr16(m, a0, (uint16_t)d1);
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x137632 dbra */
    }
    /* 0x137636: strip 4 = 0xE7 words of 0x1C00 */
    if (rd16(m, a5 - 0x7EC2u) == 0u) {               /* 0x13763A bne 0x137656 */
        a0 = 0x620632u;                              /* 0x13763E */
        d0 = 0x00E6u;                                /* 0x137644 moveq #$e6,d0 */
        d1 = 0x1C00u;                                /* 0x137648 */
        do {
            wr16(m, a0, (uint16_t)d1);               /* 0x13764C */
            a0 += 2u;
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x13764E dbra */
    } else {
        a0 = 0x6207CEu;                              /* 0x137656 */
        d0 = 0x00E6u;
        d1 = 0x1C00u;                                /* 0x137660 */
        do {
            a0 -= 2u;                                /* 0x137664 */
            wr16(m, a0, (uint16_t)d1);
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
        } while ((uint16_t)d0 != 0xFFFFu);            /* 0x137666 dbra */
    }
    /* 0x13766A: the A4 motion block */
    wr32(m, a4 + 0x0008u, 0xFF700000u);              /* 0x13766A */
    wr32(m, a4 + 0x000Cu, 0x00A00000u);              /* 0x137672 */
    wr32(m, a4 + 0x0010u, 0x00050000u);              /* 0x13767A */
    wr32(m, a4 + 0x0014u, 0x00000000u);              /* 0x137682 */
    wr16(m, a4 + 0x0018u, 0x0000u);                  /* 0x13768A */
    wr16(m, a4 + 0x001Au, 0x0000u);                  /* 0x137690 */
    wr16(m, a4 + 0x001Cu, 0x0000u);                  /* 0x137696 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[0] = a0;
    r->a[7] = sp + 4u;                               /* 0x13769C rts */
    return 0;
}

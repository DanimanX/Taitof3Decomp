/* ROM 0x13F5B8 stage outro (called from Task_StageTimeline at 0x13F3F6),
 * in segments (src/kernel/seg.h). Translated by Claude from the ROM listing.
 *
 *   13F5B8  push.w #2 / push.w #3 / jsr 0x1060E / lea 4(SP)   reset groups 3..4
 *   13F5CA  bset #3,-0x144C(A5) / bclr #4,-0x144C(A5)
 *   13F5D6  move.w #2,D0 (+1 if 0x32DE>3, +1 more if >6) / jsr 0x15D0
 *   13F5F8  move.w #0x3C,D7 / subq.w #1,D7
 *   13F5FE  trap #5 / btst #1,-0x144B(A5) / bne 13F5FE / dbra D7,13F5FE
 *   13F60C  fill 0xE8 words 0xDDDD at 0x627600 + -0x5D56(A5) (D7/A0 saved)
 *   13F62A  bclr #3,-0x144C(A5)
 *   13F630  lea 0x13F964,A0 / D0 = table[0x32DC(A5)] / Obj_Spawn(D0, 3, 2, 0)
 *   13F656  clr.w 0x3318(A5)
 *   13F65A  trap #5 / unless bit 4 of -0x144C: ++0x3318, > 0x2A30 -> 0x11BC6E
 *   13F67A  until bit 3 of -0x144B
 *   13F682  bset #2,0x1D36(A5) (+ bit 7 if 0x32DE == 7) / jsr 0x13DAD4 / rts */
#include "../kernel/seg.h"
#include "../objects/obj_pool.h"

/* 0x15D0, register-accurate: D0 = id % 6 (quotient in the high word),
 * A0 = 0x16890, D1 = the table long at 4(A0,D0.w*8), +1 in its low word when
 * positive; then the duration word and the long pointer go to A5 vars. */
static void fx_15D0(const f3mem_t *m, f3regs_t *r)
{
    wr16(m, F3_A5 - 0x5DE6u, 0u);                              /* 15D0 */
    r->a[0] = 0x00016890u;                                     /* 15D6 */
    uint32_t d0 = r->d[0] & 0xFFu;                             /* 15DC */
    d0 = ((d0 % 6u) << 16) | (d0 / 6u);                        /* 15E2 divu.w #6 */
    d0 = (d0 >> 16) | (d0 << 16);                              /* 15E6 swap */
    r->d[0] = d0;
    int16_t i = (int16_t)d0;
    uint32_t d1 = rd32(m, r->a[0] + 4u + (uint32_t)((int32_t)i * 8));  /* 1608 (ext 0604: *8) */
    r->d[1] = d1;
    if ((int32_t)d1 > 0) {                                     /* 160C ble */
        seg_setw(&r->d[1], (uint16_t)(d1 + 1u));               /* 1610 addq.w */
        wr16(m, F3_A5 - 0x5DE8u, (uint16_t)r->d[1]);           /* 1612 */
        wr32(m, F3_A5 - 0x5DECu,
             rd32(m, r->a[0] + (uint32_t)((int32_t)i * 8)));   /* 1616 */
    }
}

uint32_t Seg_13F5B8(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x0013F5B8u:
        Obj_ResetGroups(m, 3u, 2u);                            /* 13F5C0 (saves its regs) */
        wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x08u));
        wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & ~0x10u));
        seg_setw(&r->d[0], 2u);                                /* 13F5D6 */
        if ((int16_t)rd16(m, F3_A5 + 0x32DEu) > 3) {           /* 13F5DA ble */
            if ((int16_t)rd16(m, F3_A5 + 0x32DEu) > 6)         /* 13F5E4 ble */
                seg_setw(&r->d[0], (uint16_t)(r->d[0] + 1u));
            seg_setw(&r->d[0], (uint16_t)(r->d[0] + 1u));
        }
        fx_15D0(m, r);                                         /* 13F5F2 */
        seg_setw(&r->d[7], 0x3Bu);                             /* 13F5F8 #$3C, subq #1 */
        return 0x0013F5FEu;

    case 0x0013F600u:
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)                   /* 13F600 btst/bne */
            return 0x0013F5FEu;
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));          /* 13F608 dbra */
        if ((uint16_t)r->d[7] != 0xFFFFu)
            return 0x0013F5FEu;
        {   /* 13F60C movem D7/A0 ... movem: both restored */
            uint32_t a0 = 0x00627600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
            for (unsigned k = 0; k < 0xE8u; k++)
                wr16(m, a0 + 2u * k, 0xDDDDu);
        }
        wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & ~0x08u));  /* 13F62A */
        r->a[0] = 0x0013F964u;                                 /* 13F630 */
        seg_setw(&r->d[0], rd16(m, F3_A5 + 0x32DCu));          /* 13F636 */
        r->d[0] = rd32(m, r->a[0] + (uint32_t)((int32_t)(int16_t)r->d[0] * 4));
        r->d[0] = Obj_Spawn(m, r->a[0], r->d[0], 3u, 2u, 0u);  /* 13F64C (D0 = object or -1) */
        wr16(m, F3_A5 + 0x3318u, 0u);                          /* 13F656 */
        return 0x0013F65Au;

    case 0x0013F65Cu:
        if (!(rd8(m, F3_A5 - 0x144Cu) & 0x10u)) {              /* 13F65C btst/bne */
            uint16_t t = (uint16_t)(rd16(m, F3_A5 + 0x3318u) + 1u);
            wr16(m, F3_A5 + 0x3318u, t);                       /* 13F666 */
            if ((int16_t)t > 0x2A30)                           /* 13F66A ble */
                return SEG_LEAVE;  /* 0x11BC6E: registers not modelled in C */
        }
        if (!(rd8(m, F3_A5 - 0x144Bu) & 0x08u))                /* 13F67A beq */
            return 0x0013F65Au;
        wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x04u));  /* 13F682 */
        if (rd16(m, F3_A5 + 0x32DEu) == 7u)                    /* 13F688 */
            wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x80u));
        seg_push32(m, r, 0x0013F69Eu);                         /* 13F698 jsr 0x13DAD4 */
        return Seg_Run(m, r, 0x0013DAD4u);

    case 0x0013F69Eu:                                          /* 13F69E rts */
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0013F5FEu:
    case 0x0013F65Au:                                          /* resumed on a TRAP */
        return pc;
    }
    return SEG_LEAVE;
}

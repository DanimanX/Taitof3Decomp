#include "fn_auto.h"
/* ROM 0x0C8C64: queue one 8-byte draw record {1, entry ptr, word D0} into the
 * ring at A5-0x6750 (0x4018B0) through the enqueue helper 0x001660.
 * D0.w = word[0x0C9160 + (A1+0x22).w*4] + ((D1.w + (A2+2).w) << 2);
 * A0 = 0x0C8CCE + (4,A3).w*4 (those entries live just below that base) and
 * (4,A3).w is incremented. D0 keeps its high half, D1/D2 are pushed here and
 * 0x001660 saves D0-D2 plus A0/A2, so only D0.w and A0 change. */
int Fn_0C8C64(const f3mem_t *m, f3regs_t *r)
{
    /* 0x0C8C64 movem.l {D2 D1},-(SP) */
    uint32_t sp = r->a[7] - 8u;
    wr32(m, sp + 4u, r->d[2]);
    wr32(m, sp, r->d[1]);

    /* 0x0C8C68 move.w (4,A3),D2w ; 0x0C8C6C addq.w #1,(4,A3) */
    uint16_t idx3 = rd16(m, r->a[3] + 0x04u);
    wr16(m, r->a[3] + 0x04u, (uint16_t)(idx3 + 1u));
    r->d[2] = (r->d[2] & 0xFFFF0000u) | idx3;

    /* 0x0C8C70 lea (0x4EC,PC),A0 ; 0x0C8C76 move.w (0x22,A1),D0w ;
     * 0x0C8C7A move.w (2,A0,D0w*4),D0w  (index sign-extended, scale 4). */
    uint16_t idx1 = rd16(m, r->a[1] + 0x22u);
    uint16_t tab = rd16(m, 0x0C915Eu + 2u + (uint32_t)(int32_t)(int16_t)idx1 * 4u);
    /* 0x0C8C7E add.w (2,A2),D1w ; 0x0C8C82 lsl.w #2,D1w ; 0x0C8C84 add.w D1w,D0w */
    uint16_t d1w = (uint16_t)(r->d[1] + rd16(m, r->a[2] + 0x02u));
    d1w = (uint16_t)(d1w << 2);
    uint16_t d0w = (uint16_t)(tab + d1w);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;

    /* 0x0C8C86 lea (0x46,PC),A0 ; 0x0C8C8C lea (0,A0,D2w*4),A0 */
    uint32_t a0 = 0x0C8CCEu + (uint32_t)(int32_t)(int16_t)idx3 * 4u;
    r->a[0] = a0;

    /* 0x0C8C90..0x0C8CA0: move.l #1,-(SP) ; move.l D0,-(SP) ; move.l A0,-(SP) ;
     * jsr 0x001660 ; lea (0xC,SP),SP */
    wr32(m, sp - 4u, 1u);
    wr32(m, sp - 8u, r->d[0]);
    wr32(m, sp - 12u, a0);

    /* ROM 0x001660: append {word 1, long A0, word D0} to the ring when the
     * count at A5-0x5DF0 (0x402210) is below 0x20, else drop it. */
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);                    /* 0x001678 */
    if ((int16_t)n < 0x20) {                                  /* 0x00167C bge.b 0x00169E */
        uint16_t off = (uint16_t)(n << 3);                     /* 0x001682 lsl.w #3 */
        uint32_t a2 = (F3_A5 - 0x6750u) + (uint32_t)(int32_t)(int16_t)off; /* 0x001688 adda.w */
        wr16(m, a2, 1u);                                      /* 0x00168A move.w D1w,(A2) */
        wr32(m, a2 + 0x02u, a0);                              /* 0x00168C move.l A0,(2,A2) */
        wr16(m, a2 + 0x06u, d0w);                             /* 0x001690 move.w D0w,(6,A2) */
        wr16(m, F3_A5 - 0x5DEEu, 2u);                         /* 0x001694 */
        wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));         /* 0x00169A addq.w #1 */
    }

    /* 0x0C8CA4 movem.l (SP)+,{D1 D2} */
    r->d[1] = rd32(m, sp);
    r->d[2] = rd32(m, sp + 4u);
    r->a[7] = sp + 8u;
    return 0;                                                 /* 0x0C8CA8 rts */
}

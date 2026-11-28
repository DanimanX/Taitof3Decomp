/* ROM 0x004216: reset the "pending" staging block at A5+0x87A and seed it with
 * the mode byte at 0x40223C shifted left by 3:
 *
 *   004216  movem.l {D0 D1 D2 A0 A1},-(SP)
 *   00421A  clr.l A5+0x87A ; clr.l A5+0x882 ; clr.l A5+0x886 ; clr.w A5+0x88A
 *   00422A  D0 = zero-extended byte (A5-0x5DC4 = 0x40223C)
 *   004230  lsl.w #3,D0 ; move.w D0,A5+0x87A
 *   004236  movem.l (SP)+,{D0 D1 D2 A0 A1} ; rts
 *
 * Every register is restored; only those Work RAM bytes change. */
#include "fn_auto.h"

int Fn_004216(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2];
    uint32_t a0 = r->a[0], a1 = r->a[1];
    uint16_t v;

    r->a[7] = sp - 20u;                            /* 0x004216 movem.l */
    wr32(m, r->a[7] + 0u, d0);
    wr32(m, r->a[7] + 4u, d1);
    wr32(m, r->a[7] + 8u, d2);
    wr32(m, r->a[7] + 12u, a0);
    wr32(m, r->a[7] + 16u, a1);

    wr32(m, a5 + 0x087Au, 0u);                     /* 0x00421A */
    wr32(m, a5 + 0x0882u, 0u);                     /* 0x00421E */
    wr32(m, a5 + 0x0886u, 0u);                     /* 0x004222 */
    wr16(m, a5 + 0x088Au, 0u);                     /* 0x004226 */

    v = (uint16_t)((uint16_t)rd8(m, a5 - 0x5DC4u) << 3); /* 0x00422A..0x004230 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | v;
    wr16(m, a5 + 0x087Au, v);                      /* 0x004232 */

    r->d[0] = rd32(m, r->a[7] + 0u);                /* 0x004236 movem.l (SP)+ */
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->d[2] = rd32(m, r->a[7] + 8u);
    r->a[0] = rd32(m, r->a[7] + 12u);
    r->a[1] = rd32(m, r->a[7] + 16u);
    r->a[7] = sp + 4u;                              /* the rts pops the caller's PC */
    return 0;
}

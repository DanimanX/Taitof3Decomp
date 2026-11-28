#include "fn_auto.h"
/* ROM 0x0C67EA: emit the four 16x16 quadrants of an object sprite (A0 = obj)
 * into the sprite list. The centre is (A0+0x14, A0+0x18) plus (+0x2C, +0x30);
 * each quadrant is offset by ((+0x2E) - 0xF, (+0x32) - 0xF) mirrored in the
 * sign bits 24/25 of D5, and 0x0C6836 appends {D6, D0&D4, D1&D4, D5, 0} at the
 * sprite cursor A5-0xCA0 and advances it by 0x10. The emitter masks D0/D1 in
 * place, so the following add/sub work on the masked words. */
#include "../video/sprite.h"

/* ROM 0x0C6836: append one 16-byte sprite entry and advance the cursor. */
static void emit_c6836(const f3mem_t *m, uint32_t d6, uint32_t d5, uint16_t d4w,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, SPR_CURSOR);                     /* 0x0C6836 */
    wr32(m, a1, d6);                                       /* 0x0C683A move.l D6,(A1) */
    *d0_io = (*d0_io & 0xFFFF0000u) | (uint16_t)((uint16_t)*d0_io & d4w); /* 0x0C683C */
    *d1_io = (*d1_io & 0xFFFF0000u) | (uint16_t)((uint16_t)*d1_io & d4w); /* 0x0C683E */
    wr16(m, a1 + 0x04u, (uint16_t)*d0_io);                 /* 0x0C6840 */
    wr16(m, a1 + 0x06u, (uint16_t)*d1_io);                 /* 0x0C6844 */
    wr32(m, a1 + 0x08u, d5);                               /* 0x0C6848 */
    wr16(m, a1 + 0x0Cu, 0u);                               /* 0x0C684C */
    wr32(m, SPR_CURSOR, a1 + 0x10u);                       /* 0x0C6850..0x0C6854 */
    *a1_io = a1 + 0x10u;
}

int Fn_0C67EA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0], d1 = r->d[1];
    uint16_t d2w, d3w;
    uint32_t a1 = rd32(m, SPR_CURSOR);
    uint16_t d4w = (uint16_t)r->d[4];

    d0 = (d0 & 0xFFFF0000u) | rd16(m, a0 + 0x2Cu);         /* 0x0C67EA move.w (0x2c,A0),D0w */
    d1 = (d1 & 0xFFFF0000u) | rd16(m, a0 + 0x30u);         /* 0x0C67EE move.w (0x30,A0),D1w */
    d2w = (uint16_t)d0;                                    /* 0x0C67F2 */
    d3w = (uint16_t)d1;                                    /* 0x0C67F4 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 + rd16(m, a0 + 0x14u)); /* 0x0C67F6 */
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 + rd16(m, a0 + 0x18u)); /* 0x0C67FA */
    d2w = (uint16_t)(d2w - rd16(m, a0 + 0x2Eu));           /* 0x0C67FE */
    d2w = (uint16_t)(d2w + 0x000Fu);                       /* 0x0C6802 */
    d3w = (uint16_t)(d3w - rd16(m, a0 + 0x32u));           /* 0x0C6806 */
    d3w = (uint16_t)(d3w + 0x000Fu);                       /* 0x0C680A */

    r->d[5] &= ~(1u << 25);                                /* 0x0C680E bclr.l #0x19,D5 */
    emit_c6836(m, r->d[6], r->d[5], d4w, &d0, &d1, &a1);   /* 0x0C6812 bsr 0x0C6836 */

    d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 - d2w);        /* 0x0C6816 sub.w D2w,D0w */
    r->d[5] |= 1u << 24;                                   /* 0x0C6818 bset.l #0x18,D5 */
    emit_c6836(m, r->d[6], r->d[5], d4w, &d0, &d1, &a1);   /* 0x0C681C */

    d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 - d3w);        /* 0x0C6820 sub.w D3w,D1w */
    r->d[5] |= 1u << 25;                                   /* 0x0C6822 bset.l #0x19,D5 */
    emit_c6836(m, r->d[6], r->d[5], d4w, &d0, &d1, &a1);   /* 0x0C6826 */

    d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 + d2w);        /* 0x0C682A add.w D2w,D0w */
    r->d[5] &= ~(1u << 24);                                /* 0x0C682C bclr.l #0x18,D5 */
    emit_c6836(m, r->d[6], r->d[5], d4w, &d0, &d1, &a1);   /* 0x0C6830 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = (r->d[2] & 0xFFFF0000u) | d2w;               /* 0x0C67F2/0x0C6802 */
    r->d[3] = (r->d[3] & 0xFFFF0000u) | d3w;               /* 0x0C67F4/0x0C680A */
    r->a[1] = a1;                                          /* 0x0C6854 */
    return 0;                                              /* 0x0C6834 rts */
}

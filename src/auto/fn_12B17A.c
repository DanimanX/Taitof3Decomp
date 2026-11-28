#include "fn_auto.h"
/* ROM 0x12B17A: integrate a 9-step curved trail into (D0, D1). Starting from
 * the record at (A5+0x328E) it walks the +0x48 linked list nine times,
 * advancing the angle accumulator (A5+0x3288) by (A5+0x328A) (itself advanced
 * by A5+0x328C) and adding each record's speed +0x42 along the table 0x27E40
 * (sin at +0x80) with muls.w. D0/D1 start as the first record's +0x14/+0x18.
 * D3-D6 and A1/A2 are saved and restored; only D0, D1 and D2 change. */
int Fn_12B17A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a2 = rd32(m, F3_A5 + 0x328Eu);      /* 0x12B184 movea.l (0x328E,A5),A2 */
    uint32_t d0 = rd32(m, a2 + 0x14u);           /* 0x12B188 */
    uint32_t d1 = rd32(m, a2 + 0x18u);           /* 0x12B18C */
    uint16_t d2w = rd16(m, F3_A5 + 0x3288u);     /* 0x12B190 */
    uint16_t d3w = rd16(m, F3_A5 + 0x328Au);     /* 0x12B194 */
    uint16_t step = rd16(m, F3_A5 + 0x328Cu);
    uint16_t d5w;

    for (unsigned n = 9u; n != 0u; --n) {        /* 0x12B198 move.w #8,D4w ; dbf */
        a2 = rd32(m, a2 + 0x48u);                /* 0x12B19C movea.l (0x48,A2),A2 */
        d2w = (uint16_t)(d2w + d3w);             /* 0x12B1A0 add.w D3w,D2w */
        d3w = (uint16_t)(d3w + step);            /* 0x12B1A2 add.w (0x328C,A5),D3w */
        d5w = (uint16_t)(d2w >> 8);              /* 0x12B1A6 lsr.w #8 */
        int16_t sp = (int16_t)rd16(m, a2 + 0x42u);           /* 0x12B1AA */
        int16_t tr = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)(int32_t)(int16_t)d5w * 2u);
        d0 += (uint32_t)((int32_t)sp * (int32_t)tr);         /* 0x12B1AE..0x12B1B4 */
        sp = (int16_t)rd16(m, a2 + 0x42u);                   /* 0x12B1B6 */
        tr = (int16_t)rd16(m, 0x00027E40u + (uint32_t)(int32_t)(int16_t)d5w * 2u);
        d1 += (uint32_t)((int32_t)sp * (int32_t)tr);         /* 0x12B1BA..0x12B1C0 */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = (r->d[2] & 0xFFFF0000u) | d2w;     /* D2 is not saved by the movem */
    return 0;                                    /* 0x12B1C6 movem.l (SP)+ ; rts */
}

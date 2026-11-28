#include "fn_auto.h"
/* ROM 0x17855A: the mirror of 0x178502 -- step one counter of the per-row
 * grid at A5+0x42B4 back toward the table 0x1788DE. Row = ((A5+0x4AB4) +
 * (A5+0x4AB8)) * 8, column from the word table 0x1788C2 indexed by
 * (A5+0x4AB6); the cell is a word while that index stays below 6 and a byte
 * after that. A cell that differs from its mirror value is decremented. */
static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_17855A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x42B4u;                 /* 0x17855A lea (0x42B4,A5),A0 */
    uint32_t a1 = 0x001788C2u;                     /* 0x17855E lea (0x362,PC),A1 */
    uint32_t a2 = 0x001788DEu;                     /* 0x178564 lea (0x378,PC),A2 */
    uint16_t d6w = rd16(m, F3_A5 + 0x4AB4u);       /* 0x17856A */
    uint16_t d5w = rd16(m, F3_A5 + 0x4AB8u);       /* 0x17856E */
    uint16_t d4w = rd16(m, F3_A5 + 0x4AB6u);       /* 0x178572 */

    r->d[3] = setw(r->d[3], rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d4w * 2u)); /* 0x178576 */
    d6w = (uint16_t)(d6w + d5w);                   /* 0x17857A add.w D5w,D6w */
    r->d[6] = (uint32_t)((int32_t)(int16_t)d6w * 8); /* 0x17857C muls.w #8,D6 */
    a0 += (uint32_t)(int32_t)(int16_t)d6w;         /* 0x178580 lea (0,A0,D6w),A0 */
    r->d[4] = setw(r->d[4], d4w);
    r->d[5] = setw(r->d[5], d5w);

    uint32_t idx = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[3];
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[2] = a2;

    if ((int16_t)d4w >= 6) {                       /* 0x178584 bge.w 0x1785A0 */
        r->d[1] = (r->d[1] & 0xFFFFFF00u) | rd8(m, a0 + idx); /* 0x1785A0 move.b */
        if (rd8(m, a0 + idx) != rd8(m, a2 + idx))  /* 0x1785A4 cmp.b ; beq */
            wr8(m, a0 + idx, (uint8_t)(rd8(m, a0 + idx) - 1u)); /* 0x1785AC */
    } else {
        r->d[1] = setw(r->d[1], rd16(m, a0 + idx)); /* 0x17858C move.w */
        if (rd16(m, a0 + idx) != rd16(m, a2 + idx)) /* 0x178590 cmp.w ; beq */
            wr16(m, a0 + idx, (uint16_t)(rd16(m, a0 + idx) - 1u)); /* 0x178598 */
    }
    return 0;                                      /* 0x1785B0 rts */
}

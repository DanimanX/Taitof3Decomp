#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x1785B2: HUD text row writer (no arguments; A5-relative state).
 *
 * Row base is A5+0x42B4, advanced by (A5+0x4AB4 + A5+0x4AB8) * 8; the column
 * index (A5+0x4AB6, doubled) selects a word offset from the ROM tables at
 * 0x1788C2 and 0x1788CC (the low-table at +6 or more reads a byte instead of a
 * word and prints 2 digits). The value is drawn at 0x61C000 + 2*coloff +
 * (A5+0x4AB8+2)*0x80 with attribute 8 through 0x005690, i.e. 4 hex digits (or
 * 2 past the sixth column). D0 = end of the written run. */

int Fn_1785B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x42B4u;                                /* 0x1785B2 */
    uint16_t d7w = rd16(m, F3_A5 + 0x4AB4u);                      /* 0x1785C2 */
    uint16_t d6w = rd16(m, F3_A5 + 0x4AB8u);                      /* 0x1785C6 */
    uint16_t d5w = rd16(m, F3_A5 + 0x4AB6u);                      /* 0x1785CA */
    uint16_t d4w, d3w;

    d5w = (uint16_t)(d5w + d5w);                                  /* 0x1785CE */
    d4w = rd16(m, 0x001788C2u + d5w);                             /* 0x1785D0 */
    d3w = rd16(m, 0x001788CCu + d5w);                             /* 0x1785D4 */

    d7w = (uint16_t)(d7w + d6w);                                  /* 0x1785D8 */
    int32_t d7 = (int32_t)(int16_t)d7w * 8;                       /* 0x1785DA muls.w #8 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)d7;               /* 0x1785DE */

    uint32_t val;
    uint16_t digits;
    if ((int16_t)d5w >= 6) {                                      /* 0x1785E2 */
        val = (r->d[1] & 0xFFFFFF00u) | rd8(m, a0 + d4w);         /* 0x178620 move.b */
        digits = 2u;
    } else {
        val = (r->d[1] & 0xFFFF0000u) | rd16(m, a0 + d4w);        /* 0x1785EA */
        digits = 4u;
    }

    uint16_t d6 = (uint16_t)(d6w + 2u);                           /* 0x1785EE / 0x178624 */
    d6 = (uint16_t)(d6 << 7);                                     /* 0x1785F8 lsl.w #7 */
    d3w = (uint16_t)((uint16_t)(d3w + d3w) + d6);                 /* 0x1785FA..0x1785FC */
    uint32_t dst = (uint32_t)(int32_t)(int16_t)d3w + 0x0061C000u; /* 0x1785FE..0x178600 */

    r->d[0] = Gfx_DrawHex(m, val, digits, dst, 8u);               /* 0x17860E */
    r->d[1] = val;
    r->a[0] = a0;
    r->a[1] = 0x001788C2u;                                        /* 0x1785B6 */
    r->a[2] = 0x001788CCu;                                        /* 0x1785BC */
    return 0;
}

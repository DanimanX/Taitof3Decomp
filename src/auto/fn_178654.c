#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x178654: HUD text row writer, attribute-7 variant of 0x1785B2 (no
 * arguments; A5-relative state).
 *
 * Row base A5+0x42B4 + (A5+0x4AB4 + A5+0x4AB8) * 8; the doubled column index
 * (A5+0x4AB6) selects a word offset from ROM 0x1788C2 / 0x1788CC. Past column
 * 6 a byte is read and 2 digits are printed, otherwise a word and 4 digits.
 * The value goes to 0x61C000 + 2*coloff + (A5+0x4AB8+2)*0x80 with attribute 7
 * through 0x005690. D0 = end of the written run. */

int Fn_178654(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x42B4u;                                /* 0x178654 */
    uint16_t d7w = rd16(m, F3_A5 + 0x4AB4u);                      /* 0x178664 */
    uint16_t d6w = rd16(m, F3_A5 + 0x4AB8u);                      /* 0x178668 */
    uint16_t d5w = rd16(m, F3_A5 + 0x4AB6u);                      /* 0x17866C */
    uint16_t d4w, d3w;

    d5w = (uint16_t)(d5w + d5w);                                  /* 0x178670 */
    d4w = rd16(m, 0x001788C2u + d5w);                             /* 0x178672 */
    d3w = rd16(m, 0x001788CCu + d5w);                             /* 0x178676 */

    int wide = (int16_t)d5w >= 6;                                 /* 0x17867A */
    if (!wide) {
        d7w = (uint16_t)(d7w + d6w);                              /* 0x178682 */
        int32_t d7 = (int32_t)(int16_t)d7w * 8;                   /* 0x178684 muls.w #8 */
        a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)d7;           /* 0x178688 */
    }

    uint32_t val;
    uint16_t digits;
    if (wide) {
        val = (r->d[1] & 0xFFFFFF00u) | rd8(m, a0 + d4w);         /* 0x1786CC move.b */
        digits = 2u;
    } else {
        val = (r->d[1] & 0xFFFF0000u) | rd16(m, a0 + d4w);        /* 0x17868C */
        digits = 4u;
    }

    uint16_t d6 = (uint16_t)(d6w + 2u);                           /* 0x178690 / 0x1786D0 */
    d6 = (uint16_t)(d6 << 7);                                     /* 0x17869A lsl.w #7 */
    d3w = (uint16_t)((uint16_t)(d3w + d3w) + d6);                 /* 0x17869C..0x17869E */
    uint32_t dst = (uint32_t)(int32_t)(int16_t)d3w + 0x0061C000u; /* 0x1786A0..0x1786A2 */

    r->d[0] = Gfx_DrawHex(m, val, digits, dst, 7u);               /* 0x1786B0 */
    r->d[1] = val;
    r->a[0] = a0;
    r->a[1] = 0x001788C2u;                                        /* 0x178658 */
    r->a[2] = 0x001788CCu;                                        /* 0x17865E */
    return 0;
}

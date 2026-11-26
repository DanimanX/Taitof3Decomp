/* ROM 0x19D988: initialize trailing ring buffer at 0x41B8E8 + (+0x22)*0xC0
 * from parent (+0x08) position (+0x50, +0x54) + (+0x16, +0x1A) (A0 = object). */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_19D988(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x22u);           /* 0x19D98E bfextu (0x22,A0){0:8},D0 */
    d0 = (uint32_t)((uint16_t)d0 * 0x00C0u);               /* 0x19D994 mulu.w #0xc0,D0 */
    uint32_t a3 = 0x0041B8E8u + (uint32_t)(int32_t)(int16_t)d0; /* 0x19D988, 0x19D998 */
    wr32(m, obj + 0x0Cu, a3);                              /* 0x19D99C */

    uint32_t a4 = rd32(m, obj + 0x08u);                    /* 0x19D9A0 */
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)); /* 0x19D9A4, 0x19D9AC */
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)); /* 0x19D9A8, 0x19D9B0 */
    wr16(m, obj + 0x50u, x);                               /* 0x19D9B4 */
    wr16(m, obj + 0x54u, y);                               /* 0x19D9B8 */

    for (int16_t d7 = 0x0F; d7 >= 0; d7--) {               /* 0x19D9BC..0x19D9D2 */
        wr16(m, a3, x);                                    /* 0x19D9C0 */
        wr16(m, a3 + 2u, y);                               /* 0x19D9C2 */
        wr16(m, a3 + 4u, x);                               /* 0x19D9C6 */
        wr16(m, a3 + 8u, y);                               /* 0x19D9CA */
        a3 += 0x0Cu;                                       /* 0x19D9CE */
    }

    r->d[0] = setw(d0, x);
    r->d[1] = setw(r->d[1], y);
    r->d[7] = setw(r->d[7], 0xFFFFu);
    r->a[3] = a3;
    r->a[4] = a4;
    return 0;
}

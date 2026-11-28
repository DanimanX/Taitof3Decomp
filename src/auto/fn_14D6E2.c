/* ROM 0x14D6E2: repaint one line-scroll table. Sets the low nibble of the
 * 0xE8 words at 0x62B200+scroll (scroll = signed word at A5-0x5D56) to 0xA via
 * 0x17668A, with D6 = 0xE7 and D7 = 0 over the pushed words; A0, D6 and D7 are
 * saved and restored, D1.w keeps the last word rewritten. */
#include "fn_auto.h"

/* ROM 0x17668A: D1.w = (A0,D7.w*2), replace its low nibble with D0's, store
 * back, until D7 passes D6 (both words compared signed). */
static void fill_17668A(const f3mem_t *m, uint32_t a0, uint16_t d0w, uint16_t d6w,
                        uint32_t *d1_io)
{
    for (uint16_t d7 = 0u; (int16_t)d7 <= (int16_t)d6w; d7++) {   /* ROM 0x17668A..0x17669A */
        uint32_t addr = a0 + (uint32_t)d7 * 2u;
        uint16_t w = rd16(m, addr);                               /* 0x17668A */
        w = (uint16_t)((w & 0xFFF0u) | (d0w & 0x000Fu));           /* 0x17668E bfins D0,D1{28:4} */
        *d1_io = (*d1_io & 0xFFFF0000u) | w;
        wr16(m, addr, w);                                         /* 0x176692 */
    }
}

int Fn_14D6E2(const f3mem_t *m, f3regs_t *r)
{
    /* 0x14D6E2: movem.l A0/D7/D6,-(SP) saves the caller's A0/D6/D7, the two
     * pushed words load D6 = 0xE7 and D7 = 0 below, and both are popped again
     * and restored at 0x14D706, so only D0 and D1 survive. */
    int16_t scroll = (int16_t)rd16(m, F3_A5 - 0x5D56u);            /* 0x14D6EC adda.w */
    uint32_t a0 = 0x0062B200u + (uint32_t)(int32_t)scroll;         /* 0x14D6E6 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x000Au;                   /* 0x14D6F8 */
    fill_17668A(m, a0, (uint16_t)r->d[0], 0x00E7u, &r->d[1]);      /* 0x14D700 */
    return 0;                                                      /* 0x14D70A */
}

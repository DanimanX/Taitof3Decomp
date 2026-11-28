/* ROM 0x13E7EE: (injected capture fnj_, NOIRQ): set the display-window bounds in
 * graphic RAM. The capture only observes the A5+0x3300 word (0x0000 -> 0x0030 in pair 0);
 * its gfx pre/post snapshots are byte-identical, so the 0xE7-word fill is not
 * checked by the test and was verified from the bytes alone.
 * A0 = 0x620800 + the signed word at A5-0x5D56 (0x4022AA), plus one word when
 * the flag byte at A5-0x5D58 (0x4022A8) is clear. That flag also picks the
 * value written to A5+0x3300: 0x30 (clear) or 0x1CE (set). The word at A0 is
 * ORed with bit 0 and stored into 0xE7 consecutive words from A0 upward
 * (0xE820 do-while, D0 = 0xE6). D0/D1/A0 are pushed and popped, so the routine
 * returns every register unchanged. */
#include "fn_auto.h"

int Fn_13E7EE(const f3mem_t *m, f3regs_t *r)
{
    (void)r;

    uint32_t a0 = 0x00620800u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x13E7F8 */
    if (rd8(m, F3_A5 - 0x5D58u) != 0u) {                      /* 0x13E7FC tst.b */
        wr16(m, F3_A5 + 0x3300u, 0x01CEu);                    /* 0x13E810 */
    } else {
        wr16(m, F3_A5 + 0x3300u, 0x0030u);                    /* 0x13E804 */
        a0 += 2u;                                             /* 0x13E80A */
    }

    uint16_t d1 = (uint16_t)(rd16(m, a0) | 1u);               /* 0x13E81A */
    for (unsigned i = 0; i < 0xE7u; i++) {                    /* 0x13E820 dbf */
        wr16(m, a0, d1);
        a0 += 2u;
    }
    return 0;
}

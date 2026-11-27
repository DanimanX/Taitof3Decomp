/* ROM 0x13E8A2: clear bit 2 of 0xE7 consecutive words of a sprite/text table.
 * The table base is 0x620800 + the signed word at A5-0x5D56, plus 2 when the
 * byte at A5-0x5D58 is clear; that branch also stores 0x30 (vs 0x1CE) to
 * A5+0x3300. D0/D1/A0 are saved and restored. */
#include "fn_auto.h"

int Fn_13E8A2(const f3mem_t *m, f3regs_t *r)
{
    (void)r;

    uint32_t a0 = 0x00620800u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x13E8A6/0x13E8AC */
    int32_t d0;

    if (rd8(m, F3_A5 - 0x5D58u) == 0u) {   /* 0x13E8B0 tst.b ; 0x13E8B4 bne.w 0x13E8C4 */
        wr16(m, F3_A5 + 0x3300u, 0x0030u); /* 0x13E8B8 */
        a0 += 2u;                          /* 0x13E8BE addq.w #2,A0 */
    } else {
        wr16(m, F3_A5 + 0x3300u, 0x01CEu); /* 0x13E8C4 */
    }

    /* 0x13E8CA move.w #0xE6,D0 ; 0x13E8CE read, 0x13E8D0 andi.w #-5 ; then the
     * dbf loop at 0x13E8D6 targets 0x13E8D4, so the read happens ONCE and the
     * same word is stored 0xE7 times (dbf runs 0xE7 passes). */
    uint16_t d1 = (uint16_t)(rd16(m, a0) & 0xFFFBu); /* andi.w #-5,D1 */
    for (d0 = 0xE6; d0 >= 0; d0--) {
        wr16(m, a0, d1);                             /* move.w D1,(A0)+ */
        a0 += 2u;
    }
    return 0;
}

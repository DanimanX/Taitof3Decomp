/* ROM 0x176236: shift/copy signed word range [idx0..idx1] by delta words.
 * Stack args at SP+4: base.l, idx0.w, idx1.w, delta.w. Copies words from
 * base[i*2] to base[(i+delta)*2], choosing iteration direction to avoid
 * clobbering unread source words. */
#include "fn_auto.h"

int Fn_176236(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a0 = rd32(m, sp + 4u);                       /* 0x17623E (0x8,A6) */
    int16_t d7 = (int16_t)rd16(m, sp + 8u);               /* 0x176242 (0xC,A6) */
    int16_t d6 = (int16_t)rd16(m, sp + 10u);              /* 0x176246 (0xE,A6) */
    int16_t d0 = (int16_t)rd16(m, sp + 12u);              /* 0x17624A (0x10,A6) */
    int16_t d5 = 1;                                       /* 0x17624E */

    if (d0 != 0) {                                        /* 0x176252 */
        if (d0 < 0) {                                     /* 0x176258 */
            if (d7 >= d6) {                               /* 0x17625C..0x176262 */
                int16_t tmp = d6;
                d6 = d7;
                d7 = tmp;
            }
        } else {
            d5 = -1;                                      /* 0x176266 */
            if (d7 <= d6) {                               /* 0x176268..0x17626A */
                int16_t tmp = d6;
                d6 = d7;
                d7 = tmp;
            }
        }

        for (;;) {
            int16_t d4 = (int16_t)(d0 + d7);              /* 0x176270..0x176272 */
            uint16_t v = rd16(m, (uint32_t)((int32_t)a0 + (int32_t)d7 * 2));
            wr16(m, (uint32_t)((int32_t)a0 + (int32_t)d4 * 2), v); /* 0x176274 */
            d7 = (int16_t)(d7 + d5);                      /* 0x17627A */
            if (d5 < 0) {
                if (d7 < d6)
                    break;
            } else {
                if (d7 > d6)
                    break;
            }
        }
    }

    r->a[0] = a0;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0;
    return 0;
}

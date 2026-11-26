/* ROM 0x137C16: boss phase timer / transition tick (A0 = object).
 * When A5+0x32D6 < 5, decrements +0x06 and re-arms 0x137C16 while > 0;
 * on expiry, if A5+0x3318 <= 0x0BB8 sets A5+0x30EE bit 0x11 (A5+0x30F0 bit 6),
 * resets +0x06 to 0x012C and arms 0x137BB6. Otherwise latches A5+0x32D6 = 5,
 * clears A5+0x3318, sets (+0x54=2, +0x50=0x138, +0x52=0x78, +0x06=0x12C) and
 * arms 0x137C5C. */
#include "fn_auto.h"

int Fn_137C16(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) < 5) {          /* 0x137C16..0x137C1C */
        int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x137C20 */
        wr16(m, obj + 0x06u, (uint16_t)cnt);
        if (cnt > 0) {                                     /* 0x137C24 */
            wr32(m, obj, 0x00137C16u);                     /* 0x137C0E */
            return 0;
        }
        if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x0BB8) { /* 0x137C26..0x137C2C */
            wr8(m, F3_A5 + 0x30F0u, (uint8_t)(rd8(m, F3_A5 + 0x30F0u) | 0x40u)); /* 0x137BA2: bfset {0x11:1} */
            wr16(m, obj + 0x06u, 0x012Cu);                 /* 0x137BA8 */
            wr32(m, obj, 0x00137BB6u);                     /* 0x137BAE */
            return 0;
        }
        wr16(m, F3_A5 + 0x32D6u, 0x0005u);                 /* 0x137C30 */
    }

    wr16(m, F3_A5 + 0x3318u, 0u);                          /* 0x137C36 */
    wr8(m, obj + 0x54u, 0x02u);                            /* 0x137C3A */
    wr16(m, obj + 0x50u, 0x0138u);                         /* 0x137C40 */
    wr16(m, obj + 0x52u, 0x0078u);                         /* 0x137C46 */
    wr16(m, obj + 0x06u, 0x012Cu);                         /* 0x137C4C */
    /* 0x137C52: beq.b 0x137C74 never taken (0x012C != 0) */
    wr32(m, obj, 0x00137C5Cu);                             /* 0x137C54 */
    return 0;
}

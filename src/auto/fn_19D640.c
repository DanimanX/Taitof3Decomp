/* ROM 0x19D640: boss sub-part wait-for-trigger tick (A0 = object).
 * If A5+0x4B38 == -1, branches to 0x17A84C (+0x06 = 2, transitions to
 * 0x17A85C). Otherwise copies parent (+0x08 -> A4) +0x50/+0x54 to self with
 * -9 Y offset, and once A5+0x4B3A MSB is set loads script 0x19D788 into
 * +0x44/+0x48/+0x4A and transitions to 0x19D686 (else re-arms 0x19D640). */
#include "fn_auto.h"

int Fn_19D640(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {                     /* 0x19D640..0x19D646 */
        wr16(m, obj + 0x06u, 2u);                                  /* 0x17A84C (0x17A852 beq folded) */
        wr32(m, obj, 0x0017A85Cu);                                 /* 0x17A854 */
        return 0;
    }

    uint32_t a4 = rd32(m, obj + 0x08u);                            /* 0x19D64C */
    r->a[4] = a4;
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));                     /* 0x19D650 */
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) - 9u));    /* 0x19D656..0x19D65C */

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x80u) == 0u) {                 /* 0x19D662..0x19D668 */
        wr32(m, obj, 0x0019D640u);                                 /* 0x19D638 */
        return 0;
    }

    uint32_t a1 = 0x0019D788u;                                     /* 0x19D66A */
    r->a[1] = a1;
    wr32(m, obj + 0x44u, a1);                                      /* 0x19D670 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                        /* 0x19D674 */
    wr16(m, obj + 0x4Au, 0u);                                      /* 0x19D67A */
    wr32(m, obj, 0x0019D686u);                                     /* 0x19D67E */
    return 0;
}

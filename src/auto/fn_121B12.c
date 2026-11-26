/* ROM 0x121B12: stage intro camera scroll-in and boss-warning trigger (A0 = object).
 * Subtracts 0x6000 from A5+0x30D6.l; while high word > 0x40 re-arms 0x121B12,
 * else clears A5+0x30F2/30F3, plays sound 0x13, clears +0x58 on players
 * (A5+0x3264/3268), sets A5+0x30EE bit 0 (0x80), sets +0x06=0x1E, and arms 0x121B60. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_121B12(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) - 0x00006000u); /* 0x121B12 */
    if ((int16_t)rd16(m, F3_A5 + 0x30D6u) > 0x0040) {                  /* 0x121B1A..0x121B20 */
        wr32(m, obj, 0x00121B12u);                                     /* 0x121B0A */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 0u);                                       /* 0x121B22 */
    wr8(m, F3_A5 + 0x30F3u, 0u);                                       /* 0x121B26 */
    Sound_PlayHooked(m, 0x0013u);                                      /* 0x121B2A..0x121B3A */

    /* ROM 0x123630: clear +0x58 on both player objects (A5+0x3264, A5+0x3268). */
    wr8(m, rd32(m, F3_A5 + 0x3264u) + 0x58u, 0u);                      /* 0x121B3E..0x123634 */
    uint32_t a1 = rd32(m, F3_A5 + 0x3268u);                            /* 0x123638 */
    wr8(m, a1 + 0x58u, 0u);                                            /* 0x12363C */

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u)); /* 0x121B4A {0:1} */
    wr16(m, obj + 0x06u, 0x001Eu);                                     /* 0x121B50 (0x121B56 beq folded) */
    wr32(m, obj, 0x00121B60u);                                         /* 0x121B58 */

    r->d[0] &= 0xFFFFFF00u;
    r->d[1] &= 0xFFFFFF00u;
    r->a[1] = a1;
    return 0;
}

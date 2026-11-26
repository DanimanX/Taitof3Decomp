/* ROM 0x132F60: stage camera scroll-right controller tick (A0 = object).
 * Advances A5+0x30D2 by 0xC000 per frame and re-arms 0x132F60 while < 0x130.
 * Once >= 0x130, if A5+0x30DA <= 0x514 plays hooked sound 0x13, clears bit 6
 * of A5+0x30EE, sets +0x06 = 0x3C, and transitions to 0x133046; else sets
 * bits 6 and 12 of A5+0x30EE, clears A5+0x30F2, sets +0x06 = 0x168, and
 * transitions to 0x132F9C. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_132F60(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x0000C000u); /* 0x132F60 */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x0130) {        /* 0x132F68..0x132F6E */
        wr32(m, obj, 0x00132F60u);                           /* 0x132F58 */
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x0514) {       /* 0x132F70..0x132F76 */
        Sound_PlayHooked(m, 0x0013u);                        /* 0x13301C..0x13302C (0x13F238 + 0x0039D8) */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x02u)); /* 0x133030 {6:1} */
        wr16(m, obj + 0x06u, 0x003Cu);                       /* 0x133036 (beq 13304C never taken) */
        wr32(m, obj, 0x00133046u);                           /* 0x13303E */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x02u)); /* 0x132F7A {6:1} */
    wr8(m, F3_A5 + 0x30F2u, 0x00u);                          /* 0x132F80 */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x132F86 {12:1} */
    wr16(m, obj + 0x06u, 0x0168u);                           /* 0x132F8C (beq 132FA2 never taken) */
    wr32(m, obj, 0x00132F9Cu);                               /* 0x132F94 */
    return 0;
}

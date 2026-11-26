/* ROM 0x13328E: camera horizontal scroll-left tick (A0 = object).
 * Subtracts 0xC000 from A5+0x30D2.l and re-arms 0x13328E while A5+0x30D2.w
 * > 0xA0. Otherwise sets A5+0x30F2 = 8, sets bf (A5+0x30EE){12:1}
 * (0x30EF |= 8), sets +0x06 = 0x012C, and transitions to 0x1332BA. */
#include "fn_auto.h"

int Fn_13328E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x0000C000u); /* 0x13328E */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0) {        /* 0x133296..0x13329C */
        wr32(m, obj, 0x0013328Eu);                           /* 0x133286 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 8u);                             /* 0x13329E */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x1332A4 bfset {12:1} */
    wr16(m, obj + 0x06u, 0x012Cu);                           /* 0x1332AA (beq 0x1332C0 never taken) */
    wr32(m, obj, 0x001332BAu);                               /* 0x1332B2 */
    return 0;
}

/* ROM 0x132EDE: boss/camera vertical scroll-out tick (A0 = object).
 * Subtracts 2 from A5+0x30D6.w and re-arms 0x132EDE while > 0x00B1.
 * Once <= 0x00B1, sets A5+0x30F2 = 1, sets bf (A5+0x30EE){12:1} (0x30EF |= 8),
 * sets +0x06 = 0x012C, and transitions to 0x132F06. */
#include "fn_auto.h"

int Fn_132EDE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t y = (int16_t)(rd16(m, F3_A5 + 0x30D6u) - 2u);    /* 0x132EDE */
    wr16(m, F3_A5 + 0x30D6u, (uint16_t)y);
    if (y > 0x00B1) {                                        /* 0x132EE2..0x132EE8 */
        wr32(m, obj, 0x00132EDEu);                           /* 0x132ED6 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 1u);                             /* 0x132EEA */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x132EF0 bfset {12:1} */
    wr16(m, obj + 0x06u, 0x012Cu);                           /* 0x132EF6 (beq 0x132F0C never taken) */
    wr32(m, obj, 0x00132F06u);                               /* 0x132EFE */
    return 0;
}

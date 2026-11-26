/* ROM 0x13781A: stage/camera scroll phase transition tick (A0 = object).
 * Counts down +0x06 and re-arms 0x13781A while > 0. Once <= 0, sets bits
 * 1..3 of A5+0x30EE (0x70); if A5+0x3318 <= 0xBB8, copies A5+0x1D28 to
 * A5+0x30E2, sets +0x20 = 0x4000, +0x06 = 0x64, and transitions to 0x137794;
 * otherwise sets A5+0x32D6 = 2, clears A5+0x3318, sets +0x54 = 4, +0x50 = 0x2E0,
 * +0x52 = 0x80, +0x06 = 0x64, and transitions to 0x137868. */
#include "fn_auto.h"

int Fn_13781A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x13781A */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x13781E */
        wr32(m, obj, 0x0013781Au);                       /* 0x137812 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x70u)); /* 0x137820..0x13782C */

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x0BB8) {   /* 0x137832..0x137838 */
        wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u)); /* 0x13777A */
        wr16(m, obj + 0x20u, 0x4000u);                   /* 0x137780 */
        wr16(m, obj + 0x06u, 0x0064u);                   /* 0x137786 */
        wr32(m, obj, 0x00137794u);                       /* 0x13778C */
        return 0;
    }

    wr16(m, F3_A5 + 0x32D6u, 2u);                        /* 0x13783C */
    wr16(m, F3_A5 + 0x3318u, 0u);                        /* 0x137842 */
    wr8(m, obj + 0x54u, 4u);                             /* 0x137846 */
    wr16(m, obj + 0x50u, 0x02E0u);                       /* 0x13784C */
    wr16(m, obj + 0x52u, 0x0080u);                       /* 0x137852 */
    wr16(m, obj + 0x06u, 0x0064u);                       /* 0x137858 (beq 0x137880 never taken) */
    wr32(m, obj, 0x00137868u);                           /* 0x137860 */
    return 0;
}

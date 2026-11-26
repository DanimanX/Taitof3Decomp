/* ROM 0x18E718: boss fan-spread projectile attack setup tick (A0 = object).
 * Counts down +0x06 and re-arms 0x18E718 while > 0. On zero, sets +0x64=4,
 * +0x62=0x1E, +0x42=0x300, scales +0x42 by difficulty via 0x184984, and
 * branches on A5+0x4B38: if 0, sets +0x06=2 and transitions to 0x18E196; else
 * sets +0x06=4 and transitions to 0x18E750. */
#include "fn_auto.h"

/* ROM 0x184984: scale (+0x42) upward by difficulty table at 0x1849D6. */
static void sub_184984(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((rd8(m, F3_A5 + 0x087Fu) >> 4) & 0x0Fu); /* 0x184988 {8:4} */
    uint16_t pct = (uint16_t)rd8(m, 0x001849D6u + (uint32_t)idx);      /* 0x18498E (0x184992 + 0x44) */
    int32_t val = (int16_t)rd16(m, obj + 0x42u);                       /* 0x184994 */
    int16_t quot = (int16_t)(val / 0x0064);                            /* 0x18499A */
    int16_t delta = (int16_t)((int32_t)(int16_t)pct * (int32_t)quot);  /* 0x18499E */
    wr16(m, obj + 0x42u, (uint16_t)((int16_t)val + delta));            /* 0x1849A0 */
}

int Fn_18E718(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x18E718 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x18E71C */
        wr32(m, obj, 0x0018E718u);                          /* 0x18E710 */
        return 0;
    }

    wr16(m, obj + 0x64u, 4u);                               /* 0x18E71E */
    wr16(m, obj + 0x62u, 0x001Eu);                          /* 0x18E724 */
    wr16(m, obj + 0x42u, 0x0300u);                          /* 0x18E72A */
    sub_184984(m, obj);                                     /* 0x18E730 */

    if (rd16(m, F3_A5 + 0x4B38u) == 0u) {                   /* 0x18E736..0x18E73C */
        wr16(m, obj + 0x06u, 2u);                           /* 0x18E186 (beq 18E19C never taken) */
        wr32(m, obj, 0x0018E196u);                          /* 0x18E18E */
    } else {
        wr16(m, obj + 0x06u, 4u);                           /* 0x18E740 (beq 18E756 never taken) */
        wr32(m, obj, 0x0018E750u);                          /* 0x18E748 */
    }
    return 0;
}

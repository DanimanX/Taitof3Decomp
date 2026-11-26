/* ROM 0x1292BE: boss camera/script initialization delay tick (A0 = object).
 * Decrements +0x06; while > 0 re-arms 0x1292BE. On expiry initializes heading
 * +0x20 = 0x8000, target (+0x50, +0x52) = (0xC8, 0x64), clears +0x42/+0x56, sets
 * +0x54 = 3, +0x06 = 0x104, and transitions to 0x1292F4 (beq folded). */
#include "fn_auto.h"

int Fn_1292BE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x1292BE */
    wr16(m, obj + 0x06u, (uint16_t)cnt);

    if (cnt > 0) {                                          /* 0x1292C2 */
        wr32(m, obj, 0x001292BEu);                          /* 0x1292B6 */
        return 0;
    }

    wr16(m, obj + 0x20u, 0x8000u);                          /* 0x1292C4 */
    wr16(m, obj + 0x50u, 0x00C8u);                          /* 0x1292CA */
    wr16(m, obj + 0x52u, 0x0064u);                          /* 0x1292D0 */
    wr16(m, obj + 0x42u, 0u);                               /* 0x1292D6 */
    wr16(m, obj + 0x56u, 0u);                               /* 0x1292DA */
    wr8(m, obj + 0x54u, 3u);                                /* 0x1292DE */
    wr16(m, obj + 0x06u, 0x0104u);                          /* 0x1292E4 */
    /* 0x1292EA: beq.b 0x12930C never taken (0x0104 != 0) */
    wr32(m, obj, 0x001292F4u);                              /* 0x1292EC */
    return 0;
}

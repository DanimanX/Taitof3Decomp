/* ROM 0x1988F2: object step (A0 = object). Decrements (+0x06).w; while it
 * stays positive the object re-arms itself with 0x1988F2, otherwise it moves
 * on to state 0x198904. */
#include "fn_auto.h"

int Fn_1988F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1988F2 */
    wr16(m, obj + 0x06u, (uint16_t)n);

    if (n > 0)
        wr32(m, obj + 0x00u, 0x001988F2u);               /* 0x1988EA (bgt) */
    else
        wr32(m, obj + 0x00u, 0x00198904u);               /* 0x1988F8 -> 0x1988FC */
    return 0;
}

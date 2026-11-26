/* ROM 0x12D594: boss sub-part scroll-in / delay tick (A0 = object).
 * Loads A1 = (A5+0x32AA)->+0x48, adds 0x1000 to +0x56.l and subtracts
 * 0x2200 from +0x5A.l. While +0x56.w < 0x18, re-arms 0x12D594; otherwise
 * sets +0x06 = 0x3C and transitions to 0x12D5C4. */
#include "fn_auto.h"

int Fn_12D594(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u); /* 0x12D594..0x12D598 */
    r->a[1] = a1;

    wr32(m, a1 + 0x56u, rd32(m, a1 + 0x56u) + 0x00001000u);  /* 0x12D59C */
    wr32(m, a1 + 0x5Au, rd32(m, a1 + 0x5Au) - 0x00002200u);  /* 0x12D5A4 */

    if ((int16_t)rd16(m, a1 + 0x56u) < 0x0018) {             /* 0x12D5AC..0x12D5B2 */
        wr32(m, obj, 0x0012D594u);                           /* 0x12D58C */
        return 0;
    }

    wr16(m, obj + 0x06u, 0x003Cu);                           /* 0x12D5B4 (beq 0x12D5CA never taken) */
    wr32(m, obj, 0x0012D5C4u);                               /* 0x12D5BC */
    return 0;
}

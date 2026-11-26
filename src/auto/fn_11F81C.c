/* ROM 0x11F81C: boss timer tick (A0 = object).
 * Decrements +0x06; while > 0 re-arms 0x11F81C. On expiry sets A5+0x30EE bit 6
 * (0x02), calls Sound_PlayHooked(0x2F), sets +0x06 = 0x10, and transitions to
 * 0x11F84C (the subsequent beq is never taken since 0x10 != 0). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_11F81C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x11F81C */
    wr16(m, obj + 0x06u, (uint16_t)cnt);

    if (cnt > 0) {                                          /* 0x11F820 */
        wr32(m, obj, 0x0011F81Cu);                          /* 0x11F814 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x02u)); /* 0x11F822: bfset {6:1} */
    Sound_PlayHooked(m, 0x002Fu);                           /* 0x11F828..0x11F838 */
    wr16(m, obj + 0x06u, 0x0010u);                          /* 0x11F83C */
    /* 0x11F842: beq.b 0x11F852 never taken (0x0010 != 0) */
    wr32(m, obj, 0x0011F84Cu);                              /* 0x11F844 */
    return 0;
}

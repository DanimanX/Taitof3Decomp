#include "fn_auto.h"
/* ROM 0x129954: steers the camera, then sets camera flag 12 and sets a zero target speed when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129954(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129954u)) {
        wr32(m, obj, 0x129954u); /* 0x12994C */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EFu, (uint8_t)(rd8(m, r->a[5] + 0x30EFu) | 0x08u)); /* 0x12996C */
    wr8(m, obj + 0x54u, 0u); /* 0x129972 */
    wr16(m, obj + 6u, 0xB4u); /* 0x129976 */
    wr32(m, obj, 0x129986u); /* 0x12997E */
    r->a[7] += 4u;
    return 0;
}

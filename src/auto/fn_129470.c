#include "fn_auto.h"
/* ROM 0x129470: steers the camera, then sets camera flag 4 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129470(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129470u)) {
        wr32(m, obj, 0x129470u); /* 0x129468 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) | 0x08u)); /* 0x129488 */
    wr16(m, obj + 6u, 0x14u); /* 0x12948E */
    wr32(m, obj, 0x12949Eu); /* 0x129496 */
    r->a[7] += 4u;
    return 0;
}

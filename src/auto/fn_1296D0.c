#include "fn_auto.h"
/* ROM 0x1296D0: steers the camera, then clears camera flag 4 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_1296D0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x1296D0u)) {
        wr32(m, obj, 0x1296D0u); /* 0x1296C8 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & ~0x08u)); /* 0x1296E8 */
    wr16(m, obj + 6u, 0xA0u); /* 0x1296EE */
    wr32(m, obj, 0x1296FEu); /* 0x1296F6 */
    r->a[7] += 4u;
    return 0;
}

#include "fn_auto.h"
/* ROM 0x1299E8: steers the camera, then sets camera flag 8 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_1299E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x1299E8u)) {
        wr32(m, obj, 0x1299E8u); /* 0x1299E0 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EFu, (uint8_t)(rd8(m, r->a[5] + 0x30EFu) | 0x80u)); /* 0x129A00 */
    wr16(m, obj + 6u, 0x14u); /* 0x129A06 */
    wr32(m, obj, 0x129A16u); /* 0x129A0E */
    r->a[7] += 4u;
    return 0;
}

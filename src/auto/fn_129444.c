#include "fn_auto.h"
/* ROM 0x129444: steers the camera, then sets a zero target speed when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129444(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129444u)) {
        wr32(m, obj, 0x129444u); /* 0x12943C */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0u); /* 0x12945C */
    wr16(m, obj + 6u, 0x1Eu); /* 0x129460 */
    wr32(m, obj, 0x129470u); /* 0x129468 */
    r->a[7] += 4u;
    return 0;
}

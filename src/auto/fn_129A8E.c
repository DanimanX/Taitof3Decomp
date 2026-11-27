#include "fn_auto.h"
/* ROM 0x129A8E: steers the camera, then sets a zero target speed when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129A8E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129A8Eu)) {
        wr32(m, obj, 0x129A8Eu); /* 0x129A86 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0u); /* 0x129AA6 */
    wr16(m, obj + 6u, 0x1Eu); /* 0x129AAA */
    wr32(m, obj, 0x129ABAu); /* 0x129AB2 */
    r->a[7] += 4u;
    return 0;
}

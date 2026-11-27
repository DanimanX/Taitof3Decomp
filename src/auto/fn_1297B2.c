#include "fn_auto.h"
/* ROM 0x1297B2: steers the camera, then sets a zero target speed when the timer expires. */
#include "round8_camera_regs.h"

int Fn_1297B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x1297B2u)) {
        wr32(m, obj, 0x1297B2u); /* 0x1297AA */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0u); /* 0x1297CA */
    wr16(m, obj + 6u, 0x1Eu); /* 0x1297CE */
    wr32(m, obj, 0x1297DEu); /* 0x1297D6 */
    r->a[7] += 4u;
    return 0;
}

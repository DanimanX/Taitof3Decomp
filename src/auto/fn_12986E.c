#include "fn_auto.h"
/* ROM 0x12986E: steers the camera, then clears camera flag 5 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_12986E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x12986Eu)) {
        wr32(m, obj, 0x12986Eu); /* 0x129866 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & ~0x04u)); /* 0x129886 */
    wr16(m, obj + 6u, 0x32u); /* 0x12988C */
    wr32(m, obj, 0x12989Cu); /* 0x129894 */
    r->a[7] += 4u;
    return 0;
}

#include "fn_auto.h"
/* ROM 0x129ABA: steers the camera, then sets camera flag 4 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129ABA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129ABAu)) {
        wr32(m, obj, 0x129ABAu); /* 0x129AB2 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) | 0x08u)); /* 0x129AD2 */
    wr16(m, obj + 6u, 0x14u); /* 0x129AD8 */
    wr32(m, obj, 0x129AE8u); /* 0x129AE0 */
    r->a[7] += 4u;
    return 0;
}

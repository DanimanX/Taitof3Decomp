#include "fn_auto.h"
/* ROM 0x1294CC: steers the camera, then sets camera flags 0 and 3 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_1294CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x1294CCu)) {
        wr32(m, obj, 0x1294CCu); /* 0x1294C4 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) | 0x80u)); /* 0x1294E4 */
    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) | 0x10u)); /* 0x1294EA */
    wr16(m, obj + 6u, 0x12Cu); /* 0x1294F0 */
    wr32(m, obj, 0x129500u); /* 0x1294F8 */
    r->a[7] += 4u;
    return 0;
}

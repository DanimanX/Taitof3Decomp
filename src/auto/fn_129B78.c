#include "fn_auto.h"
/* ROM 0x129B78: steers the camera, then clears camera flags 4, 0 and 3 when the timer expires. */
#include "round8_camera_regs.h"

int Fn_129B78(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129B78u)) {
        wr32(m, obj, 0x129B78u); /* 0x129B70 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & ~0x08u)); /* 0x129B90 */
    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & ~0x80u)); /* 0x129B96 */
    wr8(m, r->a[5] + 0x30EEu, (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & ~0x10u)); /* 0x129B9C */
    wr16(m, obj + 6u, 0x3Cu); /* 0x129BA2 */
    wr32(m, obj, 0x129BB2u); /* 0x129BAA */
    r->a[7] += 4u;
    return 0;
}

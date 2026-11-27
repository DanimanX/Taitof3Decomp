#include "fn_auto.h"
/* ROM 0x1297DE: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_1297DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x1297DEu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x1297F0 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x1297DEu); /* 0x1297D6 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x08u)); /* 0x1297F6 */
    wr16(m, obj + 0x06u, 0x14u); /* 0x1297FC */
    /* 0x129802 BEQ to 0x129824 is never taken after MOVE of 0x14. */
    wr32(m, obj, 0x12980Cu); /* 0x129804 */
    r->a[7] += 4u;
    return 0;
}

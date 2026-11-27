#include "fn_auto.h"
/* ROM 0x12980C: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_12980C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x12980Cu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x12981E SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x12980Cu); /* 0x129804 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x04u)); /* 0x129824 */
    wr16(m, obj + 0x06u, 0x50u); /* 0x12982A */
    /* 0x129830 BEQ to 0x129852 is never taken after MOVE of 0x50. */
    wr32(m, obj, 0x12983Au); /* 0x129832 */
    r->a[7] += 4u;
    return 0;
}

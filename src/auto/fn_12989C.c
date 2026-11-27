#include "fn_auto.h"
/* ROM 0x12989C: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_12989C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x12989Cu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x1298AE SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x12989Cu); /* 0x129894 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x08u)); /* 0x1298B4 */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x80u)); /* 0x1298BA */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x10u)); /* 0x1298C0 */
    wr16(m, obj + 0x06u, 0x3Cu); /* 0x1298C6 */
    /* 0x1298CC BEQ to 0x1298EE is never taken after MOVE of 0x3C. */
    wr32(m, obj, 0x1298D6u); /* 0x1298CE */
    r->a[7] += 4u;
    return 0;
}

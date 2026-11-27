#include "fn_auto.h"
/* ROM 0x129500: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129500(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129500u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129512 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129500u); /* 0x1294F8 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x04u)); /* 0x129518 */
    wr16(m, obj + 0x06u, 0x3Cu); /* 0x12951E */
    /* 0x129524 BEQ to 0x129546 is never taken after MOVE of 0x3C. */
    wr32(m, obj, 0x12952Eu); /* 0x129526 */
    r->a[7] += 4u;
    return 0;
}

#include "fn_auto.h"
/* ROM 0x129926: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129926(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129926u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129938 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129926u); /* 0x12991E */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EFu, (uint8_t)(rd8(m, globals + 0x30EFu) | 0x20u)); /* 0x12993E */
    wr16(m, obj + 0x06u, 0x5Au); /* 0x129944 */
    /* 0x12994A BEQ to 0x12996C is never taken after MOVE of 0x5A. */
    wr32(m, obj, 0x129954u); /* 0x12994C */
    r->a[7] += 4u;
    return 0;
}

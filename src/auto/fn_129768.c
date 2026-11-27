#include "fn_auto.h"
/* ROM 0x129768: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129768(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129768u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x12977A SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129768u); /* 0x129760 */
        r->a[7] += 4u;
        return 0;
    }

    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x1F4) {
        boss_camera_change_phase(m, r, 1); /* 0x129D46 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0x03u); /* 0x12978A */
    wr16(m, obj + 0x50u, 0xD2u); /* 0x129790 */
    wr16(m, obj + 0x52u, 0x78u); /* 0x129796 */
    wr8(m, globals + 0x30F2u, 0x03u); /* 0x12979C */
    wr16(m, obj + 0x06u, 0x28u); /* 0x1297A2 */
    /* 0x1297A8 BEQ to 0x1297CA is never taken after MOVE of 0x28. */
    wr32(m, obj, 0x1297B2u); /* 0x1297AA */
    r->a[7] += 4u;
    return 0;
}

#include "fn_auto.h"
/* ROM 0x1296FE: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_1296FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x1296FEu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129710 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x1296FEu); /* 0x1296F6 */
        r->a[7] += 4u;
        return 0;
    }

    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x3E8) {
        boss_camera_change_phase(m, r, 0); /* 0x129724 */
        r->a[7] += 4u;
        return 0;
    }

    /* 0x129720 branches to the transition at 0x12941C. */
    wr8(m, obj + 0x54u, 0x03u); /* 0x12941C */
    wr16(m, obj + 0x50u, 0xD2u); /* 0x129422 */
    wr16(m, obj + 0x52u, 0x78u); /* 0x129428 */
    wr8(m, globals + 0x30F2u, 0x03u); /* 0x12942E */
    wr16(m, obj + 0x06u, 0x28u); /* 0x129434 */
    /* 0x12943A BEQ to 0x12945C is never taken after MOVE of 0x28. */
    wr32(m, obj, 0x129444u); /* 0x12943C */
    r->a[7] += 4u;
    return 0;
}

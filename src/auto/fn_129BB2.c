#include "fn_auto.h"
/* ROM 0x129BB2: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129BB2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129BB2u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129BC4 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129BB2u); /* 0x129BAA */
        r->a[7] += 4u;
        return 0;
    }

    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x1F4) {
        boss_camera_change_phase(m, r, 1); /* 0x129D46 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0x02u); /* 0x129BD4 */
    wr16(m, obj + 0x50u, 0xF0u); /* 0x129BDA */
    wr16(m, obj + 0x52u, 0x6Eu); /* 0x129BE0 */
    wr32(m, globals + 0x30E2u, rd32(m, globals + 0x1D28u)); /* 0x129BE6 */
    wr8(m, globals + 0x30F2u, 0x08u); /* 0x129BEC */
    wr16(m, obj + 0x06u, 0x3Cu); /* 0x129BF2 */
    /* 0x129BF8 BEQ to 0x129C1A is never taken after MOVE of 0x3C. */
    wr32(m, obj, 0x129C02u); /* 0x129BFA */
    r->a[7] += 4u;
    return 0;
}

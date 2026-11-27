#include "fn_auto.h"
/* ROM 0x12952E: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_12952E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x12952Eu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129540 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x12952Eu); /* 0x129526 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x08u)); /* 0x129546 */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x80u)); /* 0x12954C */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x10u)); /* 0x129552 */
    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x3E8) {
        boss_camera_change_phase(m, r, 0); /* 0x129724 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0x03u); /* 0x129562 */
    wr16(m, obj + 0x50u, 0xC8u); /* 0x129568 */
    wr16(m, obj + 0x52u, 0x78u); /* 0x12956E */
    wr8(m, globals + 0x30F2u, 0x07u); /* 0x129574 */
    wr16(m, obj + 0x06u, 0x3Cu); /* 0x12957A */
    /* 0x129580 BEQ to 0x1295A2 is never taken after MOVE of 0x3C. */
    wr32(m, obj, 0x12958Au); /* 0x129582 */
    r->a[7] += 4u;
    return 0;
}

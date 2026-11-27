#include "fn_auto.h"
/* ROM 0x1295BE: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_1295BE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x1295BEu);
    uint16_t timer = rd16(m, obj + 6u); /* 0x1295D0 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x1295BEu); /* 0x1295B6 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EFu, (uint8_t)(rd8(m, globals + 0x30EFu) & ~0x40u)); /* 0x1295D6 */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) & ~0x10u)); /* 0x1295DC */
    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x3E8) {
        boss_camera_change_phase(m, r, 0); /* 0x129724 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0x08u); /* 0x1295EC */
    wr16(m, obj + 0x50u, 0xFAu); /* 0x1295F2 */
    wr16(m, obj + 0x52u, 0x78u); /* 0x1295F8 */
    wr8(m, globals + 0x30F2u, 0x05u); /* 0x1295FE */
    wr16(m, obj + 0x06u, 0x50u); /* 0x129604 */
    /* 0x12960A BEQ to 0x12962C is never taken after MOVE of 0x50. */
    wr32(m, obj, 0x129614u); /* 0x12960C */
    r->a[7] += 4u;
    return 0;
}

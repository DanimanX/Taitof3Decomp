#include "fn_auto.h"
/* ROM 0x129C62: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129C62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129C62u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129C74 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129C62u); /* 0x129C5A */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EFu, (uint8_t)(rd8(m, globals + 0x30EFu) | 0x80u)); /* 0x129C7A */
    wr16(m, obj + 0x06u, 0x14u); /* 0x129C80 */
    /* 0x129C86 BEQ to 0x129CA8 is never taken after MOVE of 0x14. */
    wr32(m, obj, 0x129C90u); /* 0x129C88 */
    r->a[7] += 4u;
    return 0;
}

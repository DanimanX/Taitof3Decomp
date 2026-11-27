#include "fn_auto.h"
/* ROM 0x129C90: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_129C90(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x129C90u);
    uint16_t timer = rd16(m, obj + 6u); /* 0x129CA2 SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x129C90u); /* 0x129C88 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EFu, (uint8_t)(rd8(m, globals + 0x30EFu) | 0x10u)); /* 0x129CA8 */
    wr8(m, globals + 0x30EFu, (uint8_t)(rd8(m, globals + 0x30EFu) | 0x80u)); /* 0x129CAE */
    wr16(m, obj + 0x06u, 0x14u); /* 0x129CB4 */
    /* 0x129CBA BEQ to 0x129CDC is never taken after MOVE of 0x14. */
    wr32(m, obj, 0x129CC4u); /* 0x129CBC */
    r->a[7] += 4u;
    return 0;
}

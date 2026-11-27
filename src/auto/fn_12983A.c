#include "fn_auto.h"
/* ROM 0x12983A: Step the boss camera and advance its script when the timer expires. */
#include "fn_boss_camera.h"

int Fn_12983A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    boss_camera_step(m, r, 0x12983Au);
    uint16_t timer = rd16(m, obj + 6u); /* 0x12984C SUBQ.W */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x12983Au); /* 0x129832 */
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x80u)); /* 0x129852 */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x10u)); /* 0x129858 */
    wr16(m, obj + 0x06u, 0x12Cu); /* 0x12985E */
    /* 0x129864 BEQ to 0x129886 is never taken after MOVE of 0x12C. */
    wr32(m, obj, 0x12986Eu); /* 0x129866 */
    r->a[7] += 4u;
    return 0;
}

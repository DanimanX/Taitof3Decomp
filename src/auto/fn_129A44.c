#include "fn_auto.h"
/* ROM 0x129A44: steers the camera and selects the next sequence when the timer expires. */
#include "fn_sound_frames.h"
#include "round8_camera_regs.h"

int Fn_129A44(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129A44u)) {
        wr32(m, obj, 0x129A44u); /* 0x129A3C */
        r->a[7] += 4u;
        return 0;
    }

    uint32_t a5 = r->a[5];
    if ((int16_t)rd16(m, a5 + 0x30DAu) < 0x1F4) { /* 0x129A5C */
        call_hooked_sound(m, r, 0x13u, 0x129D50u, 0x129D56u); /* 0x129D46 */
        wr32(m, a5 + 0x30EEu, 0u);
        wr8(m, obj + 0x54u, 6u);
        wr16(m, obj + 0x50u, 0xFAu);
        wr16(m, obj + 0x52u, 0x78u);
        wr16(m, obj + 6u, 0x3Cu);
        wr32(m, obj, 0x129D80u); /* 0x129D78 */
    } else { /* 0x129A66 */
        wr8(m, obj + 0x54u, 3u);
        wr16(m, obj + 0x50u, 0xF0u);
        wr16(m, obj + 0x52u, 0x78u);
        wr8(m, a5 + 0x30F2u, 4u);
        wr16(m, obj + 6u, 0x28u);
        wr32(m, obj, 0x129A8Eu);
    }
    r->a[7] += 4u;
    return 0;
}

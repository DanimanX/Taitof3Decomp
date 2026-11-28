#include "fn_auto.h"

/* ROM 0x129E3E: steer the boss camera and arm a 60-frame state when its timer expires. */
#include "round8_camera_regs.h"

int Fn_129E3E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x129E3Eu)) {
        wr32(m, obj, 0x129E3Eu); /* 0x129E54 BGT to 0x129E36 */
    } else {
        wr16(m, obj + 6u, 0x003Cu); /* 0x129E56 sets Z clear */
        wr32(m, obj, 0x129E66u);   /* 0x129E5E */
    }
    r->a[7] += 4u;
    return 0;
}

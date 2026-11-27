#include "fn_auto.h"
/* ROM 0x10C818: verified: the capture discriminates (an empty body fails it): Follow the parent, relay damage, and publish collision bounds. */
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

int Fn_10C818(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 8u);
    r->a[1] = parent;
    uint8_t state = rd8(m, parent + 6u);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | state;
    if (state == 0) { /* 0x10C820 uses a signed long displacement to 0x0CEE74. */
        Obj_StartDying(m, obj);
        return 0;
    }
    if (state == 1 || (int32_t)rd32(m, parent + 0x24u) <= 0)
        return 0;

    if (rd8(m, parent + 0x35u) == 0) {
        wr8(m, parent + 0x35u, rd8(m, obj + 0x35u));
        wr32(m, parent + 0x36u, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x35u, 0u);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            wr32(m, obj + 0x24u, 0x00050000u);
            wr32(m, parent + 0x24u, 0u);
        }
    }
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    if (state != 2) {
        uint16_t index = rd16(m, parent + 0x6Au);
        r->d[0] = batch_set_word(r->d[0], index);
        /* 0x10C87C + 0x1E, indexed by signed D0.W times eight. */
        r->a[2] = 0x10C89Au + (int32_t)(int16_t)index * 8;
        for (unsigned i = 0; i < 3; ++i) {
            wr16(m, obj + 0x2Cu + i * 2u, rd16(m, r->a[2]));
            r->a[2] += 2u;
        }
        wr16(m, obj + 0x32u, rd16(m, r->a[2]));
    }

    wr32(m, r->a[7] - 4u, 0x10C898u);
    Obj_0D0DC2(m, obj);
    r->a[1] = rd32(m, r->a[5] + 0x906u);
    return 0;
}

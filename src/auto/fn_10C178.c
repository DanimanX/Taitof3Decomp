#include "fn_auto.h"
/* ROM 0x10C178: verified: the capture discriminates (an empty body fails it): follow the parent, transfer the hit state, and register collision bounds. */
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"

int Fn_10C178(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08);
    r->a[1] = parent;
    uint8_t state = rd8(m, parent + 0x06);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | state;
    if (state == 0) { /* 0x10C180: tail branch to 0x0CEE74. */
        Obj_StartDying(m, obj);
        return 0;
    }
    if (state == 1 || (int32_t)rd32(m, parent + 0x24) <= 0)
        return 0;

    if (rd8(m, parent + 0x35) == 0) { /* 0x10C19E */
        wr8(m, parent + 0x35, rd8(m, obj + 0x35));
        wr32(m, parent + 0x36, rd32(m, obj + 0x36));
        wr8(m, obj + 0x35, 0);
        if ((int32_t)rd32(m, obj + 0x24) <= 0) {
            wr32(m, obj + 0x24, 0x50000);
            wr32(m, parent + 0x24, 0);
        }
    }

    wr32(m, obj + 0x14, rd32(m, parent + 0x14));
    wr32(m, obj + 0x18, rd32(m, parent + 0x18));
    if (state != 2) {
        uint16_t index = rd16(m, parent + 0x6A);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | index;
        /* 0x10C1DA: 0x10C1DC + 0x1E with a signed D0.w index scaled by 8. */
        r->a[2] = 0x10C1FAu + (uint32_t)((int32_t)(int16_t)index * 8);
        for (uint32_t off = 0x2C; off < 0x32; off += 2) {
            wr16(m, obj + off, rd16(m, r->a[2]));
            r->a[2] += 2;
        }
        wr16(m, obj + 0x32, rd16(m, r->a[2]));
    }

    /* 0x10C1F2: A1 ends at the advanced second collision list cursor. */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x10C1F8);
    Coll_AddEnemyHazard(m, obj);
    r->a[1] = rd32(m, r->a[5] + 0x0906);
    r->a[7] += 4;
    return 0;
}

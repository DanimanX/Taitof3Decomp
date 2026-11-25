#include "obj_FEEA0.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[angle.w] scaled by D1w. */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    *out_d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
}

/* ROM 0x0FEEA0 */
int Obj_FEEA0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = rd32(m, obj + 0x08u);
    if (rd8(m, a2 + 0x06u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t idx = rd16(m, a1 + 0x00u);
    uint32_t ring_base = rd32(m, a1 + 0x04u);
    wr16(m, obj + 0x58u, rd16(m, ring_base + (uint32_t)(int32_t)(int16_t)idx));
    idx = (uint16_t)((idx + 2u) & 0x01FFu);
    wr16(m, a1 + 0x00u, idx);

    uint32_t anchor = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, anchor + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, anchor + 0x18u));

    if (anchor != a2) {
        uint16_t ang = rd16(m, obj + 0x58u);
        if ((int16_t)ang >= 0) {
            uint16_t dist = rd16(m, a2 + 0x08u);
            uint32_t dx, dy;
            sub_0D1EBC(m, ang, dist, &dx, &dy);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
        }
    }

    if (rd8(m, obj + 0x35u) != 0u) {
        wr8(m, obj + 0x35u, 0u);
        wr8(m, a2 + 0x22u, 1u);
    }

    Obj_0D0DC2(m, obj);
    return 0;
}

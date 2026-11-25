#include "obj_FE53C.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w (0..50). */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    int32_t idx = (int32_t)(int16_t)d0w * 4;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D2D64 + idx));
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2ECC + idx));
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
    return 0;
}

/* ROM 0x0FE53C */
int Obj_FE53C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t base = rd16(m, 0x000D0D82u + (uint32_t)((rd8(m, F3_A5 + 0x87Fu) >> 3) & 0x1Fu) * 2u);
    uint16_t d1w = (uint16_t)(base + 5u);
    uint16_t rng = (uint16_t)(Obj_Next4Way(m) & 0x0007u);
    wr16(m, obj + 0x60u, (uint16_t)(d1w + rng));

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    Obj_0CEDE4(m, obj, 0x000FE676u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));

    uint32_t vx, vy;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &vx, &vy) < 0)
        return -1;
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);

    Obj_StartBehaviour(m, obj, 0x000FE696u);
    uint32_t sx = rd32(m, F3_A5 + 0x4224u);
    uint32_t sy = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - sx);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - sx);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - sy);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sy);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000FE5BAu);
    return 0;
}

#include "obj_D8D30.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0C666A: OR a column of 16-bit masks {n.w, mask.w...} into the terrain
 * bitmap at 0x40891C (0x280 bits per 8-pixel row) at screen (x, y) */
static void sub_0C666A(const f3mem_t *m, uint16_t x, uint16_t y, uint32_t a1)
{
    uint32_t d0 = ((uint16_t)(x + rd16(m, F3_A5 + 0x918u)) >> 3) & 0x3FFu;
    uint32_t d1 = ((uint16_t)(y + rd16(m, F3_A5 + 0x91Au)) >> 3) & 0x3Fu;
    d0 += d1 * 0x280u;
    int16_t n = (int16_t)rd16(m, a1); a1 += 2u;
    while (--n >= 0) {
        uint32_t base = 0x0040891Cu + (d0 >> 3);
        unsigned sh = d0 & 7u;
        uint32_t w = ((uint32_t)rd8(m, base) << 16) | ((uint32_t)rd8(m, base + 1u) << 8)
                   | rd8(m, base + 2u);
        uint32_t mask = (uint32_t)rd16(m, a1) << (8u - sh);
        a1 += 2u;
        w |= mask;
        wr8(m, base, (uint8_t)(w >> 16));
        wr8(m, base + 1u, (uint8_t)(w >> 8));
        wr8(m, base + 2u, (uint8_t)w);
        d0 += 0x280u;
    }
}

static int16_t sb(const f3mem_t *m, uint32_t a) { return (int16_t)(int8_t)rd8(m, a); }

/* ROM 0x0D8D30: wall piece: record 0xD8E50[arg] {frame.l, hp.w, ?.w,
 * dx.b, dy.b, hitbox.b x4, solid.b, terrain mask...}; solid pieces mark the
 * terrain bitmap */
int Obj_D8D30_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t p = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, p + 0x14u));
    wr16(m, obj + 0x16u, 0u);
    wr16(m, obj + 0x18u, rd16(m, p + 0x18u));
    wr16(m, obj + 0x1Au, 0u);
    uint16_t idx = (uint16_t)rd32(m, obj + 0x0Cu);
    uint32_t a1 = rd32(m, 0x000D8E50u + (uint32_t)((int32_t)(int16_t)idx * 4));
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr16(m, obj + 0x24u, rd16(m, a1 + 4u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + sb(m, a1 + 8u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + sb(m, a1 + 9u)));
    wr16(m, obj + 0x2Cu, (uint16_t)sb(m, a1 + 10u));
    wr16(m, obj + 0x30u, (uint16_t)sb(m, a1 + 11u));
    wr16(m, obj + 0x2Eu, (uint16_t)sb(m, a1 + 12u));
    wr16(m, obj + 0x32u, (uint16_t)sb(m, a1 + 13u));
    uint8_t solid = rd8(m, a1 + 14u);
    wr8(m, obj + 0x34u, solid);
    a1 += 15u;
    wr32(m, obj + 0x0Cu, a1);
    if (solid != 0u)
        sub_0C666A(m, rd16(m, obj + 0x14u), rd16(m, obj + 0x18u), a1 + 1u);
    wr32(m, obj + 0x00u, 0x000D8DC4u);
    return 0;
}

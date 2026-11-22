#include "obj_101A6C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0D0E3A: LCG step at A5+0x1D60 (n = n*5+1); returns n >> 8 */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}
/* ROM 0x0D0E52: octant angle of (D0.w, D1.w) in 0x168 units */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = (d1 <= d0) ? d0 : d1;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    d0 = (uint16_t)rd8(m, 0x0D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x0D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x101A6C */
int Obj_101A6C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x101A6C: boss dying / no ammo -> dying */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x21u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x101A82 */
    uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c);
    if (c != 0u)
        return 0;

    /* 0x101A8A */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, rd8(m, obj + 0x4Eu));

    /* 0x101A94: Obj_Spawn(obj, 0x101B20, group 5, n 2, arg 0) */
    uint32_t child = Obj_Spawn(m, obj, 0x00101B20u, 5u, 2u, 0u);
    if ((int32_t)child < 0)
        return 0;

    uint32_t a1 = child;
    wr16(m, a1 + 0x14u, 0x0140u);

    uint16_t d7 = (uint16_t)(sub_0D0E3A(m) & 0x7Fu);
    uint16_t d0 = (uint16_t)(sub_0D0E3A(m) & 0x3Fu);
    d0 = (uint16_t)(d0 + d7 + 0x1Du);
    wr16(m, a1 + 0x18u, d0);

    d7 = (uint16_t)(sub_0D0E3A(m) & 0x7Fu);
    d0 = (uint16_t)(sub_0D0E3A(m) & 0x3Fu);
    d0 = (uint16_t)(d0 + d7 + 0x1Du);

    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x18u) - d0);
    uint16_t ang = sub_0D0E52(m, 0x0140u, d1);
    wr16(m, a1 + 0x58u, ang);

    d0 = (uint16_t)((sub_0D0E3A(m) & 0x3u) + 5u);
    wr16(m, a1 + 0x60u, d0);
    return 0;
}

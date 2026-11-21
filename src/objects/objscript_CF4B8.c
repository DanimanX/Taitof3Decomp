#include "objscript_CF4B8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0D0E52 */
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

    uint16_t src = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned sh = (unsigned)(d3 & 63u);
        d0 = (sh >= 16u) ? 0u : (uint16_t)(d0 >> sh);
        d1 = (sh >= 16u) ? 0u : (uint16_t)(d1 >> sh);
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    d1 = (uint16_t)(d1 + d0);
    d0 = (uint16_t)rd8(m, 0x000D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x000D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D2D44 */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: scale (D0, D1) by D2w. */
    if (d2w > 50u) {
        /* unimplemented: ROM 0x0D4F26 */
        return -1;
    }

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0CF53E */
static int sub_0CF53E(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1) < 0)
        return -1;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);

    uint32_t rem_x = rd32(m, obj + 0x50u) - d0;
    wr32(m, obj + 0x50u, rem_x);
    if ((int32_t)d0 >= 0) {
        if ((int32_t)rem_x < 0) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            return 0;
        }
    } else {
        if ((int32_t)rem_x >= 0) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            return 0;
        }
    }

    uint32_t rem_y = rd32(m, obj + 0x54u) - d1;
    wr32(m, obj + 0x54u, rem_y);
    if ((int32_t)d1 >= 0) {
        if ((int32_t)rem_y < 0)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    } else {
        if ((int32_t)rem_y >= 0)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    }
    return 0;
}

/* ROM 0x0CF590 */
static int sub_0CF590(const f3mem_t *m, uint32_t obj)
{
    uint32_t d2 = rd32(m, obj + 0x5Cu);
    uint32_t ang32 = rd32(m, obj + 0x58u) + d2;
    wr32(m, obj + 0x58u, ang32);

    if ((int32_t)ang32 < 0) {
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) + 0x0168u));
    } else if (rd16(m, obj + 0x58u) >= 0x0168u) {
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) - 0x0168u));
    }

    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1) < 0)
        return -1;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);

    uint32_t rem;
    if ((int32_t)d2 >= 0)
        rem = rd32(m, obj + 0x50u) - d2;
    else
        rem = rd32(m, obj + 0x50u) + d2;
    wr32(m, obj + 0x50u, rem);
    if ((int32_t)rem < 0)
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    return 0;
}

/* ROM 0x0CF5EE */
static int sub_0CF5EE(const f3mem_t *m, uint32_t obj)
{
    uint32_t ang32 = rd32(m, obj + 0x58u) + rd32(m, obj + 0x5Cu);
    wr32(m, obj + 0x58u, ang32);

    if ((int32_t)ang32 < 0) {
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) + 0x0168u));
    } else if (rd16(m, obj + 0x58u) >= 0x0168u) {
        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) - 0x0168u));
    }

    uint32_t spd32 = rd32(m, obj + 0x60u) + rd32(m, obj + 0x64u);
    wr32(m, obj + 0x60u, spd32);

    if ((int32_t)spd32 < 0) {
        wr32(m, obj + 0x60u, 0u);
    } else if (rd16(m, obj + 0x60u) > 0x0032u) {
        wr16(m, obj + 0x60u, 0x0032u);
    }

    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1) < 0)
        return -1;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x50u) - 1u);
    wr16(m, obj + 0x50u, cnt);
    if (cnt == 0u)
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    return 0;
}

/* ROM 0x0CF660 */
static int sub_0CF660(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t a2 = 0x0041B190u;
    uint32_t a3 = 0x0041B21Au;
    int target_found = 1;

    if ((int16_t)rd16(m, a2 + 0x4Cu) < 0) {
        if ((int16_t)rd16(m, a3 + 0x4Cu) < 0) {
            target_found = 0;
        } else {
            a2 = a3;
        }
    } else if ((int16_t)rd16(m, a3 + 0x4Cu) >= 0) {
        if (a1 == 0u) {
            /* unimplemented: ROM 0x0CF660 */
            return -1;
        }
        uint16_t dx1 = (uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a2 + 0x14u));
        dx1 = (uint16_t)(dx1 + rd16(m, a1) - 0x0017u);
        if ((int16_t)dx1 < 0)
            dx1 = (uint16_t)(0u - dx1);

        uint16_t dy1 = (uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a2 + 0x18u));
        dy1 = (uint16_t)(dy1 + rd16(m, a1 + 2u) - 0x000Fu);
        if ((int16_t)dy1 < 0)
            dy1 = (uint16_t)(0u - dy1);

        uint16_t dist1 = (dy1 > dx1) ? dy1 : dx1;

        uint16_t dx2 = (uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a3 + 0x14u));
        dx2 = (uint16_t)(dx2 + rd16(m, a1) - 0x0017u);
        if ((int16_t)dx2 < 0)
            dx2 = (uint16_t)(0u - dx2);

        uint16_t dy2 = (uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a3 + 0x18u));
        dy2 = (uint16_t)(dy2 + rd16(m, a1 + 2u) - 0x000Fu);
        if ((int16_t)dy2 < 0)
            dy2 = (uint16_t)(0u - dy2);

        uint16_t dist2 = (dy2 > dx2) ? dy2 : dx2;

        if (dist2 < dist1)
            a2 = a3;
    }

    if (target_found) {
        if (a1 == 0u) {
            /* unimplemented: ROM 0x0CF660 */
            return -1;
        }
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, a2 + 0x14u));
        dx = (uint16_t)(dx + rd16(m, a1) - 0x0017u);

        uint16_t dy = (uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a2 + 0x18u));
        dy = (uint16_t)(dy + rd16(m, a1 + 2u) - 0x000Fu);

        uint16_t target_ang = sub_0D0E52(m, dx, dy);
        uint16_t diff = (uint16_t)(target_ang - rd16(m, obj + 0x58u));
        if ((int16_t)diff < 0)
            diff = (uint16_t)(diff + 0x0168u);

        if (diff <= 0x00B4u) {
            if (diff <= rd16(m, a1 + 4u)) {
                wr16(m, obj + 0x58u, target_ang);
            } else {
                uint32_t ang32 = rd32(m, obj + 0x58u) + rd32(m, a1 + 4u);
                wr32(m, obj + 0x58u, ang32);
                if (rd16(m, obj + 0x58u) >= 0x0168u)
                    wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) - 0x0168u));
            }
        } else {
            diff = (uint16_t)((uint16_t)(0u - diff) + 0x0168u);
            if (diff <= rd16(m, a1 + 4u)) {
                wr16(m, obj + 0x58u, target_ang);
            } else {
                uint32_t ang32 = rd32(m, obj + 0x58u) - rd32(m, a1 + 4u);
                wr32(m, obj + 0x58u, ang32);
                if ((int32_t)ang32 < 0)
                    wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) + 0x0168u));
            }
        }
    }

    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1) < 0)
        return -1;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x50u) - 1u);
    wr16(m, obj + 0x50u, cnt);
    if (cnt == 0u)
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    return 0;
}

/* ROM 0x0CF7AA */
static int sub_0CF7AA(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x50u) - 1u);
    wr16(m, obj + 0x50u, cnt);
    if (cnt == 0u)
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    return 0;
}

/* ROM 0x0CF4B8 */
int ObjScript_CF4B8(const f3mem_t *m, uint32_t obj)
{
    uint16_t op = (uint16_t)rd8(m, obj + 0x20u);
    switch (op) {
    case 0u: case 1u: case 2u: case 3u: case 4u: case 5u:
    case 6u: case 7u: case 8u: case 9u: case 10u: case 11u:
    case 16u: case 17u: case 18u: case 19u:
    case 21u: case 22u: case 23u: case 24u:
    case 25u: case 26u: case 27u: case 28u:
        /* ROM 0x0CF53C */
        return 0;
    case 12u:
        return sub_0CF53E(m, obj);
    case 13u:
        return sub_0CF590(m, obj);
    case 14u:
        return sub_0CF5EE(m, obj);
    case 15u:
        return sub_0CF660(m, obj, 0u);
    case 20u:
        return sub_0CF7AA(m, obj);
    default:
        /* unimplemented: ROM 0x0CF4C8 */
        return -1;
    }
}

#include "obj_DACDC.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D0CB0 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    /* 0x0D0CBA + 0x00C8 = 0x0D0D82 */
    return rd16(m, 0x000D0D82u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
}

/* ROM 0x0D2D44 */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0DADAA */
static void state_0(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x06u) == 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }

    wr32(m, obj + 0x3Au, rd32(m, parent + 0x14u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x58u)));
    wr32(m, obj + 0x3Eu, rd32(m, parent + 0x18u));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x60u)));

    uint8_t z = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u);
    wr8(m, obj + 0x1Eu, z);
    wr16(m, obj + 0x32u, (uint16_t)(0x10u - (z >> 4)));
    if (z == 0u) {
        wr16(m, obj + 0x32u, 0x000Du);
        wr8(m, obj + 0x06u, 1u);
    }
}

static int player_y_near(const f3mem_t *m, uint32_t obj, uint32_t player)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return 0;
    int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - rd16(m, player + 0x18u) - 7);
    if (dy < 0)
        dy = (int16_t)-dy;
    return (uint16_t)dy <= 9u;
}

/* ROM 0x0DAE08 */
static void state_1(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Eu,
         rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u));
    uint32_t x3a = rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x3Au, x3a);
    if ((int32_t)x3a < 0)
        return;

    uint32_t target = 0x0041B190u;
    if (!player_y_near(m, obj, target)) {
        target = 0x0041B21Au;
        if (!player_y_near(m, obj, target))
            return;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - rd16(m, target + 0x14u) - 0x10);
    Obj_DrawSprite(m, obj);
    wr8(m, obj + 0x06u, (dx < 0) ? 3u : 2u);
}

/* ROM 0x0DAEAC / 0x0DAF86 */
static void state_split(const f3mem_t *m, uint32_t obj, int flip_x)
{
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint8_t z = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u);
    wr8(m, obj + 0x1Eu, z);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (z >> 4)));
    wr32(m, obj + 0x10u, 0x0005A588u);
    Obj_DrawSprite(m, obj);

    uint8_t nz = (uint8_t)(~rd8(m, obj + 0x1Eu));
    wr8(m, obj + 0x1Fu, nz);
    wr8(m, obj + 0x1Eu, 0u);
    if (!flip_x) {
        uint16_t off = (uint16_t)(0x10u - (nz >> 4) - 9u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - off));
    } else {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 7u));
    }
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
    if (flip_x)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
    wr32(m, obj + 0x10u, 0x0005A5C8u);
    Obj_DrawSprite(m, obj);
    if (flip_x)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & (uint8_t)~0x02u));

    uint8_t d0b = (uint8_t)(~rd8(m, obj + 0x1Fu));
    wr8(m, obj + 0x1Eu, d0b);
    wr8(m, obj + 0x1Fu, 0u);
    if (d0b != 0xF0u)
        return;

    uint16_t spd = (uint16_t)(sub_0D0CB0(m) + 0x14u);
    uint32_t vx = 0u, vy = 0u;
    sub_0D2D44(m, flip_x ? 0x0000u : 0x00B4u, spd, &vx, &vy);
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);
    if (!flip_x) {
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 8u));
    } else {
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 7u));
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 8u));
    wr8(m, obj + 0x1Eu, 0u);
    wr8(m, obj + 0x06u, 4u);
    wr16(m, obj + 0x2Cu, 2u);
    wr16(m, obj + 0x2Eu, 0x0Du);
    wr16(m, obj + 0x30u, 6u);
    wr16(m, obj + 0x32u, 9u);
}

/* ROM 0x0DACDC */
int Obj_DACDC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    switch (rd8(m, obj + 0x06u)) {
    case 0u: state_0(m, obj); break;
    case 1u: state_1(m, obj); break;
    case 2u: state_split(m, obj, 0); break;
    case 3u: state_split(m, obj, 1); break;
    case 4u:
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
        wr32(m, obj + 0x3Eu,
             rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u));
        break;
    default:
        break;
    }

    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 7u);
    int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 8u);
    if (Terrain_TestPoint(m, tx, ty) != 0) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Coll_AddHazard(m, obj);

    uint8_t st = rd8(m, obj + 0x06u);
    if (st != 2u && st != 3u)
        Obj_DrawSprite(m, obj);

    return 0;
}

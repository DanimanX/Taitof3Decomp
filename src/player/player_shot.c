#include "player_shot.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

#define SHOT_LIST_CURSOR (F3_A5 + 0x0902u)
#define SHOT_X_BOUNDS    0x000CC112u

/* ROM 0x0C9C62 */
static void sub_0C9C62(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x000C9C6Au);
}

/* ROM 0x0CA514 */
static int sub_0CA514(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0)
        return 0;
    uint32_t a1 = rd32(m, obj + 0x36u);
    int8_t grp = (int8_t)rd8(m, a1 + 0x04u);
    if (grp < 3 || grp >= 5)
        return 0;
    /* unimplemented: ROM 0x11819A */
    return -1;
}

/* ROM 0x0C9C6A */
int Obj_PlayerShot_Tick(const f3mem_t *m, uint32_t obj)
{
    if (sub_0CA514(m, obj) < 0)
        return -1;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
        /* unimplemented: ROM 0x0CA970 */
        return -1;
    }

    uint8_t shot_type = rd8(m, obj + 0x22u);
    if (shot_type != 2u && shot_type != 3u) {
        int16_t tx = (int16_t)(uint16_t)(rd16(m, obj + 0x2Eu) + rd16(m, obj + 0x14u));
        uint16_t ysum = (uint16_t)(rd16(m, obj + 0x32u) + rd16(m, obj + 0x30u));
        uint16_t ymid = (uint16_t)((ysum >> 1) | (ysum & 0x8000u));
        int16_t ty = (int16_t)(uint16_t)(ymid + rd16(m, obj + 0x18u));
        if (Terrain_TestPoint(m, tx, ty) != 0) {
            uint32_t parent = rd32(m, obj + 0x08u);
            wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
            /* unimplemented: ROM 0x0CA970 */
            return -1;
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x48u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x4Cu));

    if ((int16_t)rd16(m, obj + 0x14u) < 0x136) {
        uint32_t p = rd32(m, SHOT_LIST_CURSOR);
        wr32(m, p, obj);
        wr32(m, SHOT_LIST_CURSOR, p + 4u);
    }

    Obj_Animate(m, obj, rd32(m, obj + 0x44u));
    Obj_DrawSprite(m, obj);

    uint8_t hit = rd8(m, obj + 0x35u);
    wr8(m, obj + 0x54u, hit);
    if (hit != 0) {
        uint32_t d0 = rd32(m, obj + 0x28u);
        uint32_t half = (d0 >> 1) | (d0 & 0x80000000u);
        wr32(m, obj + 0x28u, d0 - half);
        wr8(m, obj + 0x35u, 0);
    } else if (rd8(m, obj + 0x54u) != 0) {
        uint32_t d0 = rd32(m, obj + 0x50u);
        d0 = (d0 >> 1) | (d0 & 0x80000000u);
        wr32(m, obj + 0x50u, d0);
        wr32(m, obj + 0x28u, d0);
    }

    uint8_t zoom = rd8(m, obj + 0x1Eu);
    if (zoom != 0) {
        uint32_t spr = rd32(m, obj + 0x10u);
        uint16_t d0 = rd16(m, spr);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
        d0 = (uint16_t)(d0 << 1);
        wr16(m, obj + 0x32u, (uint16_t)(rd16(m, obj + 0x32u) + d0));
        if (zoom >= 0x20u)
            wr8(m, obj + 0x1Eu, (uint8_t)(zoom - 0x20u));
        else
            wr8(m, obj + 0x1Eu, 0);
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t lo = (int16_t)rd16(m, SHOT_X_BOUNDS);
    int16_t hi = (int16_t)rd16(m, SHOT_X_BOUNDS + 2u);
    if (x < lo || x > hi) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
        Obj_Free(m, obj);
        return 0;
    }

    sub_0C9C62(m, obj);
    return 0;
}

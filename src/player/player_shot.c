#include "player_shot.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"  /* Sound_PlayHooked */

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

/* ROM 0x0CA970: the shot dies. D0 = 0 (spent, quiet puff), 1 (spent,
 * sound 0xF) or -1 (hit terrain, no sound). Pin the explosion point
 * (+0x44/+0x48 = position + hitbox centre), pick its animation, angle,
 * speed and flip, then 0x0CAB22: clear +0x1E/+0x3A/+0x3E, next tick
 * 0x0CAB36. */
static int sub_0CA970(const f3mem_t *m, uint32_t obj, uint8_t d0b)
{
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + rd16(m, obj + 0x2Eu)));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u));
    d1 = (uint16_t)(d1 >> 1);                               /* lsr.w */
    wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) + d1));

    uint32_t tbl;
    uint16_t ang;
    uint16_t spd;
    if (d0b == 0u) {                                        /* 0x0CA992 tst.w */
        tbl = 0x000CAAA8u;
        ang = (uint16_t)(uint8_t)((Rng_NextMem(m) & 0x0Fu) + 0xF8u);
        spd = 0x0028u;
    } else {
        if ((int8_t)d0b >= 0)                               /* 0x0CA9D2 tst.b ; blt */
            Sound_PlayHooked(m, 0x000Fu);
        tbl = 0x000CAACEu;
        ang = (uint16_t)(uint8_t)((Rng_NextMem(m) & 0x3Fu) + 0x60u);
        spd = 0x001Au;
    }
    wr32(m, obj + 0x36u, tbl);
    wr16(m, obj + 0x20u, ang);                              /* andi.w ; addi.b */
    wr16(m, obj + 0x42u, spd);
    wr8(m, obj + 0x1Cu, (uint8_t)(Rng_NextMem(m) & 0x02u));
    wr8(m, obj + 0x1Du, 0x00u);                             /* sf */
    wr8(m, obj + 0x4Cu, 0xFFu);                             /* st */

    wr16(m, obj + 0x1Eu, 0u);                               /* 0x0CAB22 */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x000CAB36u);                      /* 0x0CAB2E */
    return 0;
}

/* ROM 0x0C9C6A */
int Obj_PlayerShot_Tick(const f3mem_t *m, uint32_t obj)
{
    if (sub_0CA514(m, obj) < 0)
        return -1;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x0C9D4C: D0 = bfextu (0x34,(0x36,A0)){6:1} = bit 1 of that byte */
        uint8_t d0b = (uint8_t)((rd8(m, rd32(m, obj + 0x36u) + 0x34u) >> 1) & 1u);
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
        return sub_0CA970(m, obj, d0b);
    }

    uint8_t shot_type = rd8(m, obj + 0x22u);
    if (shot_type != 2u && shot_type != 3u) {
        int16_t tx = (int16_t)(uint16_t)(rd16(m, obj + 0x2Eu) + rd16(m, obj + 0x14u));
        uint16_t ysum = (uint16_t)(rd16(m, obj + 0x32u) + rd16(m, obj + 0x30u));
        uint16_t ymid = (uint16_t)((ysum >> 1) | (ysum & 0x8000u));
        int16_t ty = (int16_t)(uint16_t)(ymid + rd16(m, obj + 0x18u));
        if (Terrain_TestPoint(m, tx, ty) != 0) {
            uint32_t parent = rd32(m, obj + 0x08u);       /* 0x0C9D5A moveq #-1 */
            wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
            return sub_0CA970(m, obj, 0xFFu);
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

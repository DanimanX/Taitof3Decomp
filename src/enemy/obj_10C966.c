#include "obj_10C966.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CF7BC: arm script when X in [-64, 335] and Y in [-64, 247]. */
static void sub_0CF7BC(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (y < -0x40 || y > 0x00F7)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((rd8(m, obj + 0x07u) & ~0x80u) | 0x02u));
}

/* ROM 0x0D0C20: rotate angle.w|frac.w (d0) toward target d2w by step d1. */
static uint32_t sub_0D0C20(uint32_t d0, uint32_t d1, uint16_t d2w)
{
    uint16_t d0w = (uint16_t)(d0 >> 16);
    uint16_t d1w = (uint16_t)(d1 >> 16);
    uint16_t d3w = (uint16_t)(d2w - d0w);
    if ((int16_t)d3w < 0)
        d3w = (uint16_t)(d3w + 0x0168u);

    if (d3w > 0x00B4u) {
        d3w = (uint16_t)(0x0168u - d3w);
        if (d3w <= d1w)
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        d0 -= d1;
        if ((int32_t)d0 < 0)
            d0 += 0x01680000u;
        return d0;
    }
    if (d3w <= d1w)
        return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
    d0 += d1;
    if (d0 >= 0x01680000u)
        d0 -= 0x01680000u;
    return d0;
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[angle.w] * scale.w */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[angle.w] * scale.w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07E4: aim at nearest live player within cone/range (no screen bounds). */
static void sub_0D07E4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t p68 = rd32(m, obj + 0x68u);
    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    int16_t best_ang[2] = { -1, -1 };
    uint16_t max_dx[2] = { 0, 0 };
    uint16_t max_dy[2] = { 0, 0 };

    uint16_t r_min = rd16(m, a1 + 0x08u);
    uint16_t r_max = rd16(m, a1 + 0x0Au);
    uint16_t a_min = rd16(m, a1 + 0x0Cu);
    uint16_t a_max = rd16(m, a1 + 0x0Eu);

    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t sdy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) +
                                rd16(m, p68 + 2u) - 0x0F);
        uint16_t ady = (uint16_t)(sdy < 0 ? -sdy : sdy);
        if (ady > r_max)
            continue;
        int16_t sdx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) +
                                rd16(m, p68) - 0x17);
        uint16_t adx = (uint16_t)(sdx < 0 ? -sdx : sdx);
        if (adx > r_max)
            continue;
        if (sdx < 0) {
            if (adx < r_min && ady < r_min)
                continue;
        } else {
            if ((int16_t)adx < (int16_t)(r_min - 0x1E) && ady < r_min)
                continue;
        }
        uint16_t ang = Angle360(m, (uint16_t)sdx, (uint16_t)sdy);
        if (a_min > a_max) {
            if (ang < a_min && ang > a_max)
                continue;
        } else {
            if (ang < a_min || ang > a_max)
                continue;
        }
        best_ang[i] = (int16_t)ang;
        max_dx[i] = adx;
        max_dy[i] = ady;
    }

    int16_t chosen = best_ang[1];
    if (best_ang[0] >= 0) {
        if (best_ang[1] < 0) {
            chosen = best_ang[0];
        } else {
            uint16_t d0 = (max_dx[0] >= max_dy[0]) ? max_dx[0] : max_dy[0];
            uint16_t d1 = (max_dx[1] >= max_dy[1]) ? max_dx[1] : max_dy[1];
            if (d1 >= d0)
                chosen = best_ang[0];
        }
    } else if (best_ang[1] < 0) {
        return;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t thr = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    if ((int16_t)thr > (int16_t)Obj_Next4Way(m))
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

static uint16_t turn_cw(uint16_t ang, uint16_t delta)
{
    ang = (uint16_t)(ang + delta);
    if (ang >= 0x0168u)
        ang = (uint16_t)(ang - 0x0168u);
    return ang;
}

static uint16_t turn_ccw(uint16_t ang, uint16_t delta)
{
    int16_t s = (int16_t)(ang - delta);
    if (s < 0)
        s = (int16_t)(s + 0x0168);
    return (uint16_t)s;
}

static uint16_t turn_by_dir(uint16_t ang, uint8_t dir, uint16_t delta)
{
    return (dir == 0u) ? turn_ccw(ang, delta) : turn_cw(ang, delta);
}

static void update_probes(const f3mem_t *m, uint32_t obj, uint16_t ang)
{
    uint32_t d0, d1;
    sub_0D1EBC(m, ang, 8u, &d0, &d1);
    wr32(m, obj + 0x58u, d0);
    wr32(m, obj + 0x5Cu, d1);
    sub_0D1EBC(m, ang, 0x0Au, &d0, &d1);
    wr32(m, obj + 0x60u, d0);
    wr32(m, obj + 0x64u, d1);
}

/* LAB_0010CDAE: bounce/reverse when bit0 of obj+0x4E is clear. */
static void sub_10CDAE(const f3mem_t *m, uint32_t obj)
{
    uint8_t prev = rd8(m, obj + 0x22u);
    wr8(m, obj + 0x22u, (uint8_t)(prev ^ 0x01u));
    uint32_t a1 = ((prev & 0x01u) != 0u) ? 0x0010D0A6u : 0x0010D0E2u;
    Obj_SetAnim(m, obj, a1);
    uint16_t ang = turn_ccw(rd16(m, obj + 0x44u), 0x00B4u);
    wr16(m, obj + 0x44u, ang);
    uint32_t d0, d1;
    sub_0D2D44(m, ang, 0x0Au, &d0, &d1);
    wr32(m, obj + 0x08u, d0);
    wr32(m, obj + 0x0Cu, d1);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
}

/* LAB_0010CFD8: finish cornering and return to state 3 (or 1). */
static void sub_10CFD8(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint32_t vx, vy;
    sub_0D2D44(m, d0, 0x0Au, &vx, &vy);
    wr32(m, obj + 0x08u, vx);
    wr32(m, obj + 0x0Cu, vy);
    uint16_t ang = turn_by_dir(rd16(m, obj + 0x42u),
                               (uint8_t)(rd8(m, obj + 0x22u) == 0u ? 1u : 0u),
                               0x005Au);
    wr16(m, obj + 0x42u, ang);
    update_probes(m, obj, ang);
    wr8(m, obj + 0x06u, 3u);
    int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 0x10 + (int16_t)rd16(m, obj + 0x60u));
    int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x10 + (int16_t)rd16(m, obj + 0x64u));
    if ((uint8_t)Terrain_TestPoint(m, tx, ty) == 0u)
        wr8(m, obj + 0x06u, 1u);
}

/* ROM 0x10C966 */
int Obj_10C966_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Score_AddCurrent(m, 0x000001F4u);
            Obj_0CEF0E(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x000001F4u);
        goto lab_10c9fe;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_10c9fe;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    switch (rd8(m, obj + 0x06u)) {
    case 0: {
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x08u) - 1u);
        wr16(m, obj + 0x08u, cnt);
        if (cnt == 0u)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        sub_0CF7BC(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    }
    case 1: {
        int16_t bx = (int16_t)(rd16(m, obj + 0x3Au) + 0x10);
        int16_t by = (int16_t)(rd16(m, obj + 0x3Eu) + 0x10);
        static const int8_t dx_dir[4] = { 1, 0, -1, 0 };
        static const int8_t dy_dir[4] = { 0, -1, 0, 1 };
        static const uint16_t dir_ang[4] = { 0u, 0x005Au, 0x00B4u, 0x010Eu };
        uint16_t dists[4];
        for (int dir = 0; dir < 4; dir++) {
            uint16_t d3 = 8u;
            int found = 0;
            for (int step = 0; step < 20; step++) {
                int16_t tx = (int16_t)(bx + dx_dir[dir] * (int16_t)d3);
                int16_t ty = (int16_t)(by + dy_dir[dir] * (int16_t)d3);
                if ((uint8_t)Terrain_TestPoint(m, tx, ty) != 0u) {
                    found = 1;
                    break;
                }
                d3 = (uint16_t)(d3 + 8u);
            }
            dists[dir] = found ? d3 : 0xFFFFu;
            wr16(m, obj + 0x58u + (uint32_t)dir * 4u, dists[dir]);
        }
        uint16_t best_d = dists[0];
        uint16_t best_a = dir_ang[0];
        for (int dir = 1; dir < 4; dir++) {
            if (dists[dir] < best_d) {
                best_d = dists[dir];
                best_a = dir_ang[dir];
            }
        }
        if ((int16_t)best_d < 0)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        wr16(m, obj + 0x42u, best_a);
        wr8(m, obj + 0x06u, 2u);
        uint32_t vx, vy;
        sub_0D2D44(m, best_a, 0x0Fu, &vx, &vy);
        wr32(m, obj + 0x08u, vx);
        wr32(m, obj + 0x0Cu, vy);
        update_probes(m, obj, best_a);
        break;
    }
    case 2: {
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x08u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x0Cu));
        int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 0x10 + (int16_t)rd16(m, obj + 0x58u));
        int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x10 + (int16_t)rd16(m, obj + 0x5Cu));
        if ((uint8_t)Terrain_TestPoint(m, tx, ty) != 0u) {
            uint16_t d0 = turn_by_dir(rd16(m, obj + 0x42u), rd8(m, obj + 0x22u), 0x005Au);
            wr16(m, obj + 0x44u, d0);
            uint32_t vx, vy;
            sub_0D2D44(m, d0, 0x0Au, &vx, &vy);
            wr32(m, obj + 0x08u, vx);
            wr32(m, obj + 0x0Cu, vy);
            wr8(m, obj + 0x06u, 3u);
        }
        break;
    }
    case 3: {
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x08u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x0Cu));
        uint16_t px = (uint16_t)(rd16(m, obj + 0x3Au) + 0x10u + rd16(m, obj + 0x60u));
        uint16_t py = (uint16_t)(rd16(m, obj + 0x3Eu) + 0x10u + rd16(m, obj + 0x64u));
        wr16(m, obj + 0x50u, px);
        wr16(m, obj + 0x54u, py);
        if ((uint8_t)Terrain_TestPoint(m, (int16_t)px, (int16_t)py) != 0u) {
            uint32_t ox, oy;
            sub_0D1EBC(m, rd16(m, obj + 0x44u), 8u, &ox, &oy);
            int16_t fx = (int16_t)((int16_t)(ox >> 16) + (int16_t)rd16(m, obj + 0x3Au) + 0x10);
            int16_t fy = (int16_t)((int16_t)(oy >> 16) + (int16_t)rd16(m, obj + 0x3Eu) + 0x10);
            if ((uint8_t)Terrain_TestPoint(m, fx, fy) != 0u) {
                /* 0x10CDF4 */
                if ((rd8(m, obj + 0x4Eu) & 0x01u) == 0u) {
                    sub_10CDAE(m, obj);
                } else {
                    uint16_t a44 = turn_by_dir(rd16(m, obj + 0x44u), rd8(m, obj + 0x22u), 0x005Au);
                    wr16(m, obj + 0x44u, a44);
                    uint32_t vx, vy;
                    sub_0D2D44(m, a44, 0x0Au, &vx, &vy);
                    wr32(m, obj + 0x08u, vx);
                    wr32(m, obj + 0x0Cu, vy);
                    uint16_t a42 = turn_by_dir(rd16(m, obj + 0x42u), rd8(m, obj + 0x22u), 0x005Au);
                    wr16(m, obj + 0x42u, a42);
                    update_probes(m, obj, a42);
                }
                break;
            }
            if (rd8(m, obj + 0x23u) != 0u) {
                uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                wr8(m, obj + 0x23u, c);
                if (c != 0u)
                    break;
            }
            int16_t sx = (int16_t)rd16(m, obj + 0x3Au);
            int16_t sy = (int16_t)rd16(m, obj + 0x3Eu);
            if (sx < 0x20 || sx > 0x0120 || sy < -0x10 || sy > 0xD8)
                break;
            uint16_t saved42 = rd16(m, obj + 0x42u);
            sub_0D07E4(m, obj, 0x0010D06Eu);
            wr16(m, obj + 0x42u, saved42);
            if (rd8(m, obj + 0x23u) != 0u) {
                uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
                if ((int32_t)a1 >= 0) {
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au));
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu));
                    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
                    wr16(m, a1 + 0x10u, 0u);
                    uint16_t r = (uint16_t)((Rng_Mix1D60(m) & 0x7Fu) - 0x40u + rd16(m, obj + 0x42u));
                    int16_t sr = (int16_t)(r - 0xB4u);
                    if (sr < 0)
                        sr = (int16_t)(sr + 0x0168);
                    else if ((uint16_t)sr >= 0x0168u)
                        sr = (int16_t)(sr - 0x0168);
                    wr16(m, a1 + 0x58u, (uint16_t)sr);
                }
            }
        } else {
            /* 0x10CD6C */
            if ((rd8(m, obj + 0x4Eu) & 0x01u) == 0u) {
                sub_10CDAE(m, obj);
            } else {
                uint16_t a46 = turn_by_dir(rd16(m, obj + 0x44u),
                                           (uint8_t)(rd8(m, obj + 0x22u) == 0u ? 1u : 0u),
                                           0x005Au);
                wr16(m, obj + 0x46u, a46);
                wr8(m, obj + 0x06u, 4u);
            }
        }
        break;
    }
    case 4: {
        wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u));
        wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u));
        uint32_t d0 = sub_0D0C20(((uint32_t)rd16(m, obj + 0x44u) << 16),
                                 0x000F0000u, rd16(m, obj + 0x46u));
        wr16(m, obj + 0x44u, (uint16_t)(d0 >> 16));

        uint16_t probe_a = turn_by_dir(rd16(m, obj + 0x42u), rd8(m, obj + 0x22u), 0x005Au);
        uint32_t ox, oy;
        sub_0D1EBC(m, probe_a, 8u, &ox, &oy);
        int16_t fx = (int16_t)((int16_t)(ox >> 16) + (int16_t)rd16(m, obj + 0x3Au) + 0x10);
        int16_t fy = (int16_t)((int16_t)(oy >> 16) + (int16_t)rd16(m, obj + 0x3Eu) + 0x10);
        if ((uint8_t)Terrain_TestPoint(m, fx, fy) != 0u) {
            uint16_t a42 = turn_by_dir(rd16(m, obj + 0x42u), rd8(m, obj + 0x22u), 0x00B4u);
            wr16(m, obj + 0x42u, a42);
            uint16_t a46 = turn_by_dir(rd16(m, obj + 0x46u), rd8(m, obj + 0x22u), 0x00B4u);
            wr16(m, obj + 0x46u, a46);
            wr16(m, obj + 0x44u, a46);
            sub_10CFD8(m, obj, a46);
        } else {
            uint16_t a44 = rd16(m, obj + 0x44u);
            uint16_t ta = turn_by_dir(a44, rd8(m, obj + 0x22u), 0x005Au);
            sub_0D1EBC(m, ta, 8u, &ox, &oy);
            wr16(m, obj + 0x3Au, (uint16_t)((uint16_t)(ox >> 16) + rd16(m, obj + 0x50u) - 0x0010u));
            wr16(m, obj + 0x3Eu, (uint16_t)((uint16_t)(oy >> 16) + rd16(m, obj + 0x54u) - 0x0010u));
            if (rd16(m, obj + 0x44u) == rd16(m, obj + 0x46u))
                sub_10CFD8(m, obj, rd16(m, obj + 0x44u));
        }
        break;
    }
    default:
        return -1;
    }

    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Obj_AnimStep(m, obj);
    Coll_AddEnemyHazard(m, obj);
    Obj_HitFlash(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;

lab_10c9fe:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, 0u);
            wr16(m, a1 + 0x60u, 0u);
        }
        Obj_StartDying(m, obj);
        return 0;
    }
}

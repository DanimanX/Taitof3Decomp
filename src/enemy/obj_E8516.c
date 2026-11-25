#include "obj_E8516.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0D0C8C */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07B4 */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t muzzle = rd32(m, obj + 0x68u);
    int16_t mx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, muzzle));
    if (mx < 0x28 || mx > 0x0127)
        return;
    int16_t my = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, muzzle + 2u));
    if (my < 0x10 || my > 0xDF)
        return;

    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    uint16_t adx[2] = { 0u, 0u };
    uint16_t ady[2] = { 0u, 0u };
    uint16_t angs[2] = { 0xFFFFu, 0xFFFFu };
    uint16_t near = rd16(m, a1 + 0x08u);
    uint16_t reach = rd16(m, a1 + 0x0Au);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);

    for (int p = 0; p < 2; p++) {
        uint32_t pl = players[p];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;

        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) + rd16(m, a4) - 0x000Fu);
        uint16_t a3w = dy;
        if ((int16_t)dy < 0)
            dy = (uint16_t)(0u - dy);
        ady[p] = dy;
        if (dy > reach)
            continue;

        a4 -= 2u;
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) + rd16(m, a4) - 0x0017u);
        uint16_t a4w = dx;
        if ((int16_t)dx < 0) {
            dx = (uint16_t)(0u - dx);
            adx[p] = dx;
            if (dx > reach)
                continue;
            if (dx < near && dy < near)
                continue;
        } else {
            adx[p] = dx;
            if (dx > reach)
                continue;
            uint16_t lim = (uint16_t)(near - 0x001Eu);
            if ((int16_t)dx < (int16_t)lim && dy < near)
                continue;
        }

        uint16_t ang = Angle360(m, a4w, a3w);
        if (lo > hi) {
            if (!(ang >= lo || ang <= hi))
                continue;
        } else {
            if (ang < lo || ang > hi)
                continue;
        }
        angs[p] = ang;
    }

    uint16_t d4 = angs[0];
    uint16_t d7 = angs[1];
    uint16_t d0 = d7;

    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return;
    } else if ((int16_t)d7 < 0) {
        d0 = d4;
    } else {
        uint16_t d2 = (ady[0] > adx[0]) ? ady[0] : adx[0];
        uint16_t d5 = (ady[1] > adx[1]) ? ady[1] : adx[1];
        if (d5 >= d2)
            d0 = d4;
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    uint16_t roll = Rng_Mix1D60(m);
    if ((int16_t)d1 > (int16_t)roll)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

static void obj_E8516_spawn_at_offset(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    uint16_t idx = rd8(m, obj + 0x22u);
    uint32_t tbl = 0x000E8934u + (uint32_t)idx * 4u;
    uint16_t ox = rd16(m, tbl);
    uint16_t oy = rd16(m, tbl + 2u);
    uint32_t child = Obj_Spawn(m, obj, fn, 9u, 2u, 0u);
    if ((int32_t)child >= 0) {
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + ox));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + oy));
    }
}

static uint32_t obj_E8516_spawn_burst(const f3mem_t *m, uint32_t obj,
                                      uint32_t px, uint32_t py, uint16_t angle)
{
    uint32_t child = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
    if ((int32_t)child < 0)
        return child;
    wr32(m, child + 0x14u, px);
    wr32(m, child + 0x18u, py);
    wr16(m, child + 0x10u, 0u);
    wr16(m, child + 0x58u, angle);
    return child;
}

/* ROM 0x0E8516 */
int Obj_E8516_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x0000012Cu);
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }

        if (rd8(m, obj + 0x06u) != 2u) {
            uint8_t d0 = (uint8_t)(rd8(m, obj + 0x21u) + 1u);
            wr8(m, obj + 0x21u, d0);
            uint16_t idx = (uint16_t)((d0 >> 2) & 0x0Fu);
            wr8(m, obj + 0x22u, (uint8_t)idx);
            Obj_SetFrame(m, obj, rd32(m, 0x000E8974u + (uint32_t)idx * 4u));
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

        switch (rd8(m, obj + 0x06u)) {
        case 0u:
            Obj_ArmOnScreen(m, obj);
            if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
                wr8(m, obj + 0x06u, 1u);
            }
            break;
        case 1u:
            if (rd32(m, obj + 0x68u) != 0u) {
                sub_0D07B4(m, obj, 0x000E88C4u);
                if (rd8(m, obj + 0x23u) != 0u) {
                    wr8(m, obj + 0x23u, 0u);
                    uint16_t idx = rd8(m, obj + 0x22u);
                    Obj_SetAnim(m, obj, rd32(m, 0x000E89E6u + (uint32_t)idx * 4u));
                    wr8(m, obj + 0x06u, 2u);
                    obj_E8516_spawn_at_offset(m, obj, 0x000E8BEAu);
                }
            }
            break;
        case 2u:
            Obj_AnimStep(m, obj);
            if (rd16(m, obj + 0x4Cu) == 0u) {
                wr8(m, obj + 0x23u, 0x0Au);
                wr8(m, obj + 0x06u, 3u);
            } else {
                int16_t d1 = (int16_t)rd16(m, obj + 0x68u);
                if (d1 >= 0) {
                    uint32_t child = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
                    if ((int32_t)child >= 0) {
                        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0018u));
                        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x002Cu));
                        wr16(m, child + 0x58u, (uint16_t)d1);
                        wr16(m, child + 0x10u, 0x0019u);
                    }
                }
            }
            break;
        case 3u: {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, c);
            if (c == 0u)
                wr8(m, obj + 0x06u, 1u);
            break;
        }
        default:
            return -1;
        }

        if ((uint8_t)Rng_Mix1D60(m) == 0u)
            obj_E8516_spawn_at_offset(m, obj, 0x000E8B26u);

        Coll_AddEnemyHazard(m, obj);
        Obj_HitFlash(m, obj);
        Obj_DrawSprite(m, obj);

        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x >= -0x0A && x <= 0x011C) {
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
                Score_AddCurrent(m, 0x0000012Cu);
                wr32(m, obj + 0x10u, 0x0005E104u);
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 4u));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u));
                Obj_OrbitDebug(m, obj);
                return 0;
            }
        }
        return 0;
    }

    uint16_t idx = rd8(m, obj + 0x22u);
    uint32_t tbl = 0x000E8934u + (uint32_t)idx * 4u;
    wr32(m, obj + 0x68u, tbl);
    uint32_t px = ((uint32_t)rd16(m, tbl) << 16) + rd32(m, obj + 0x14u);
    uint32_t py = ((uint32_t)rd16(m, tbl + 2u) << 16) + rd32(m, obj + 0x18u);

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        wr8(m, obj + 0x23u, 0u);
        sub_0D07B4(m, obj, 0x000E88FCu);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint16_t base_ang = rd16(m, obj + 0x42u);
            if ((int32_t)obj_E8516_spawn_burst(m, obj, px, py, base_ang) < 0) {
                Obj_StartDying(m, obj);
                return 0;
            }
            uint16_t ang_m = (uint16_t)(base_ang - 0x0014u);
            if ((int16_t)ang_m < 0)
                ang_m = (uint16_t)(ang_m + 0x0168u);
            if ((int32_t)obj_E8516_spawn_burst(m, obj, px, py, ang_m) < 0) {
                Obj_StartDying(m, obj);
                return 0;
            }
            uint16_t ang_p = (uint16_t)(base_ang + 0x0014u);
            if (ang_p >= 0x0168u)
                ang_p = (uint16_t)(ang_p - 0x0168u);
            if ((int32_t)obj_E8516_spawn_burst(m, obj, px, py, ang_p) < 0) {
                Obj_StartDying(m, obj);
                return 0;
            }
        }
    }

    uint32_t death_fx = Obj_Spawn(m, obj, 0x000E8CAEu, 9u, 2u, 0u);
    if ((int32_t)death_fx >= 0) {
        wr32(m, death_fx + 0x14u, px);
        wr32(m, death_fx + 0x18u, py);
    }
    Obj_StartDying(m, obj);
    return 0;
}

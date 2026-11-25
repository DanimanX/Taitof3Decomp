#include "obj_FB584.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D0090: select script/behaviour from pointer table A1[d0]. */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    uint16_t off = rd16(m, 0x000D00B4u + (uint32_t)d0 * 2u);
    uint32_t ptr = rd32(m, a1 + (int16_t)off);
    Obj_StartBehaviour(m, obj, ptr);
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07B4: aim at nearest live player within bounds/angle window. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t p68 = rd32(m, obj + 0x68u);
    int16_t cx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, p68));
    int16_t cy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, p68 + 2u));
    if (cx < 0x28 || cx > 0x127 || cy < 0x10 || cy > 0xDF)
        return;

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
        uint16_t ang = Obj_Atan256(m, sdx, sdy);
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

/* ROM 0x0D0BBC: step facing angle (+0x08) toward target angle (+0x42). */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t target = rd16(m, obj + 0x42u);
    int16_t diff = (int16_t)(target - rd16(m, obj + 0x08u));
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);

    uint16_t step_w = rd16(m, a1 + 0x04u);
    uint32_t step_l = rd32(m, a1 + 0x04u);
    if ((uint16_t)diff > 0x00B4u) {
        uint16_t inv = (uint16_t)(0x0168 - diff);
        if (inv <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        int32_t cur = (int32_t)(rd32(m, obj + 0x08u) - step_l);
        if (cur < 0)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
        else
            wr32(m, obj + 0x08u, (uint32_t)cur);
    } else {
        if ((uint16_t)diff <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + step_l);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
    }
}

static void fb584_set_walk_right(const f3mem_t *m, uint32_t obj, uint32_t a4, int16_t xoff)
{
    wr16(m, a4 + 0x18u, (uint16_t)(xoff + 0x0120));
    wr16(m, obj + 0x42u, 0x0017u);
    wr32(m, obj + 0x08u, 0x00170000u);
    Obj_StartScript(m, obj, 0x000FBDCAu);
    Obj_StartBehaviour(m, obj, rd16(m, a4 + 0x14u) ? 0x000FC536u : 0x000FC4EEu);
    wr16(m, a4 + 0x14u, 0x0003u);
    wr16(m, a4 + 0x0Cu, 0x0007u);
    wr8(m, obj + 0x06u, 0x01u);
}

static void fb584_set_walk_left(const f3mem_t *m, uint32_t obj, uint32_t a4, int16_t xoff)
{
    wr16(m, a4 + 0x18u, (uint16_t)(xoff + 0x0080));
    wr16(m, obj + 0x42u, 0x009Du);
    wr32(m, obj + 0x08u, 0x009D0000u);
    Obj_StartScript(m, obj, 0x000FBD76u);
    Obj_StartBehaviour(m, obj, rd16(m, a4 + 0x14u) ? 0x000FC5BAu : 0x000FC46Au);
    wr16(m, a4 + 0x14u, 0x0002u);
    wr16(m, a4 + 0x0Cu, 0x0004u);
    wr8(m, obj + 0x06u, 0x01u);
}

static void fb584_probe_wall(const f3mem_t *m, uint32_t obj, uint32_t a4,
                             int16_t dx, uint32_t climb_script, uint32_t climb_behav,
                             uint8_t next_state)
{
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + dx);
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + 0x10);
    uint16_t h = 0u;
    wr16(m, a4 + 0x1Cu, 0u);
    for (int i = 0; i < 10; i++) {
        if ((uint8_t)Terrain_TestPoint(m, x, y) == 0)
            break;
        h++;
        y = (int16_t)(y - 8);
    }
    if (h > 3u) {
        wr16(m, a4 + 0x1Cu, h);
        Obj_StartScript(m, obj, climb_script);
        Obj_StartBehaviour(m, obj, climb_behav);
        wr8(m, obj + 0x06u, next_state);
    }
}

static void fb584_land_check(const f3mem_t *m, uint32_t obj, uint32_t a4,
                             uint32_t stand_behav, uint16_t walk_dir,
                             uint32_t script_tbl, uint32_t climb_behav, uint8_t climb_state)
{
    if (rd8(m, obj + 0x07u) & 0x02u) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x02u));
        ObjScript_CF1E4(m, obj);
    }
    ObjScript_CF4B8(m, obj, 0x000FBD06u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 0x10);
    int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + 0x1C);
    if ((uint8_t)Terrain_TestPoint(m, tx, ty) == 0)
        return;

    uint16_t h = rd16(m, a4 + 0x1Cu);
    if (h == 0u) {
        Obj_StartBehaviour(m, obj, stand_behav);
        wr16(m, a4 + 0x14u, walk_dir);
        wr16(m, a4 + 0x0Cu, 0u);
        wr8(m, obj + 0x06u, 0x01u);
    } else {
        uint32_t scr = rd32(m, script_tbl + (uint32_t)(uint16_t)(h - 4u) * 4u);
        Obj_StartScript(m, obj, scr);
        Obj_StartBehaviour(m, obj, climb_behav);
        wr8(m, obj + 0x06u, climb_state);
        wr16(m, a4 + 0x1Cu, 0u);
    }
}

static void fb584_die_explosion(const f3mem_t *m, uint32_t obj)
{
    uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
    if ((int32_t)child >= 0) {
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x10u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x10u));
        wr16(m, child + 0x58u, 0u);
        wr16(m, child + 0x60u, 0u);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0FB584 */
int Obj_FB584_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) &&
        (int16_t)rd16(m, obj + 0x14u) >= -0x0A &&
        (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        Obj_RelayTimer(m, 0x00000190u);
        Obj_0CEF0E(m, obj);
        return 0;
    }

    uint32_t a4 = rd32(m, obj + 0x0Cu);
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x00000190u);
        fb584_die_explosion(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        fb584_die_explosion(m, obj);
        return 0;
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    if ((rd8(m, obj + 0x07u) & 0x10u) == 0) {
        uint8_t d0b = rd8(m, obj + 0x07u);
        Obj_ArmOnScreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0) {
            wr8(m, obj + 0x07u, (uint8_t)((d0b & 0x7Fu) | 0x10u));
            Obj_FlagOffscreen(m, obj);
            if (rd8(m, obj + 0x07u) & 0x08u) {
                Obj_StartDying(m, obj);
                return 0;
            }
        }
    } else {
        Obj_FlagOffscreen(m, obj);
        if (rd8(m, obj + 0x07u) & 0x08u) {
            Obj_StartDying(m, obj);
            return 0;
        }
    }

    if (rd16(m, a4) != 0)
        wr16(m, a4, (uint16_t)(rd16(m, a4) - 1u));

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0:
        if ((rd8(m, obj + 0x07u) & 0x80u) && (int16_t)rd16(m, obj + 0x14u) < 0) {
            int16_t xoff = (int16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20);
            fb584_set_walk_right(m, obj, a4, (int16_t)(xoff - 0x60));
            break;
        }
        if (rd16(m, a4) == 0) {
            if (Obj_Next4Way(m) >= 0x80u) {
                fb584_set_walk_left(m, obj, a4, -0x00E4);
                break;
            }
            wr16(m, a4, 0x0E10u);
            wr16(m, a4 + 0x10u, 0u);
        }
        {
            uint16_t thr = rd16(m, 0x000FBCE2u + (uint32_t)rd16(m, a4 + 0x10u) * 2u);
            if (Obj_Next4Way(m) < thr) {
                wr16(m, a4 + 0x04u, 0x001Eu);
                wr8(m, obj + 0x06u, 0x02u);
            } else {
                int16_t xoff = (int16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20);
                if ((int16_t)rd16(m, obj + 0x14u) >= 0x00A0)
                    fb584_set_walk_left(m, obj, a4, xoff);
                else
                    fb584_set_walk_right(m, obj, a4, xoff);
            }
        }
        break;

    case 1:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0) {
            uint16_t next = rd16(m, a4 + 0x0Cu);
            wr8(m, obj + 0x06u, (uint8_t)next);
            uint32_t ptr = rd32(m, 0x000FC20Eu + (uint32_t)next * 4u);
            if (ptr != 0u)
                Obj_StartBehaviour(m, obj, ptr);
        }
        break;

    case 2:
        wr16(m, a4 + 0x04u, (uint16_t)(rd16(m, a4 + 0x04u) - 1u));
        if (rd16(m, a4 + 0x04u) == 0) {
            wr8(m, obj + 0x06u, 0x00u);
            break;
        }
        if (rd8(m, obj + 0x23u) == 0) {
            wr32(m, a4 + 0x24u, rd32(m, obj + 0x68u));
            wr32(m, obj + 0x68u, 0x000FCBC0u);
            sub_0D07B4(m, obj, 0x000FBD06u);
            a4 = rd32(m, obj + 0x0Cu);
            wr32(m, obj + 0x68u, rd32(m, a4 + 0x24u));
            uint16_t aim = rd16(m, obj + 0x42u);
            if (aim >= 0x50u && aim <= 0x64u) {
                wr8(m, obj + 0x23u, 0u);
                break;
            }
            sub_0D0BBC(m, obj, 0x000FBD06u);
            int16_t d2 = (int16_t)(rd16(m, obj + 0x42u) - 0x5A);
            if (d2 >= 0x00B4)
                d2 = -1;
            int16_t d0 = (int16_t)(rd16(m, obj + 0x08u) - 0x5A);
            if (d0 >= 0x00B4)
                d0 = -1;
            if ((d0 ^ d2) < 0) {
                wr8(m, obj + 0x23u, 0u);
                wr16(m, a4 + 0x0Cu, 0x0002u);
                wr8(m, obj + 0x06u, 0x01u);
                if (d0 < 0) {
                    Obj_StartBehaviour(m, obj, 0x000FC566u);
                    wr16(m, obj + 0x42u, 0x009Du);
                    wr32(m, obj + 0x08u, 0x009D0000u);
                    wr16(m, a4 + 0x14u, 0u);
                } else {
                    Obj_StartBehaviour(m, obj, 0x000FC49Au);
                    wr16(m, obj + 0x42u, 0x0017u);
                    wr32(m, obj + 0x08u, 0x00170000u);
                    wr16(m, a4 + 0x14u, 0x0001u);
                }
                break;
            }
            Obj_0CFA7C(m, obj, 0x000FC246u, rd16(m, obj + 0x08u));
            if (rd8(m, obj + 0x23u) == 0)
                break;
        }
        {
            uint8_t c23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, c23);
            if (c23 == 0) {
                sub_0D0090(m, obj, 0x000FC2E6u, rd16(m, obj + 0x08u));
                wr16(m, a4 + 0x08u, 0x003Cu);
                wr8(m, obj + 0x06u, 0x03u);
                Sound_PlayHooked(m, 0x0027u);
            }
        }
        break;

    case 3:
        wr16(m, a4 + 0x08u, (uint16_t)(rd16(m, a4 + 0x08u) - 1u));
        if (rd16(m, a4 + 0x08u) == 0) {
            wr8(m, obj + 0x06u, 0x00u);
            break;
        }
        Obj_StepScript(m, obj);
        {
            uint32_t a2 = rd32(m, obj + 0x68u) + 0x0Bu;
            if (rd8(m, a2++) != 0) {
                uint32_t child = Obj_Spawn(m, obj, 0x000D936Au, 9u, 2u, 0u);
                if ((int32_t)child >= 0) {
                    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + rd16(m, a2)));
                    a2 += 2u;
                    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + rd16(m, a2)));
                    a2 += 2u;
                    wr16(m, child + 0x58u, rd16(m, a2));
                    wr8(m, obj + 0x4Fu, 0x70u);
                }
            }
        }
        break;

    case 4:
    case 7:
    case 10:
    case 12:
        Obj_StepScript(m, obj);
        if (rd8(m, obj + 0x07u) & 0x02u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x02u));
            ObjScript_CF1E4(m, obj);
        }
        ObjScript_CF4B8(m, obj, 0x000FBD06u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        break;

    case 5:
        Obj_StepScript(m, obj);
        if (rd8(m, obj + 0x07u) & 0x02u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x02u));
            ObjScript_CF1E4(m, obj);
        }
        ObjScript_CF4B8(m, obj, 0x000FBD06u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        if ((int16_t)rd16(m, a4 + 0x18u) < (int16_t)rd16(m, obj + 0x14u)) {
            fb584_probe_wall(m, obj, a4, -0x10, 0x000FBDACu, 0x000FC64Au, 0x06u);
        } else {
            Obj_StartScript(m, obj, 0x000FBDACu);
            Obj_StartBehaviour(m, obj, 0x000FC64Au);
            wr8(m, obj + 0x06u, 0x06u);
        }
        break;

    case 6:
        fb584_land_check(m, obj, a4, 0x000FC662u, 0u, 0x000FBD3Eu, 0x000FC61Au, 0x0Au);
        break;

    case 8:
        Obj_StepScript(m, obj);
        if (rd8(m, obj + 0x07u) & 0x02u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x02u));
            ObjScript_CF1E4(m, obj);
        }
        ObjScript_CF4B8(m, obj, 0x000FBD06u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        if ((int16_t)rd16(m, a4 + 0x18u) > (int16_t)rd16(m, obj + 0x14u)) {
            fb584_probe_wall(m, obj, a4, 0x30, 0x000FBE00u, 0x000FC6DAu, 0x09u);
        } else {
            Obj_StartScript(m, obj, 0x000FBE00u);
            Obj_StartBehaviour(m, obj, 0x000FC6DAu);
            wr8(m, obj + 0x06u, 0x09u);
        }
        break;

    case 9:
        fb584_land_check(m, obj, a4, 0x000FC6F2u, 1u, 0x000FBD5Au, 0x000FC6AAu, 0x0Cu);
        break;

    case 11:
        Obj_StartBehaviour(m, obj, 0x000FC64Au);
        wr8(m, obj + 0x06u, 0x06u);
        break;

    case 13:
        Obj_StartBehaviour(m, obj, 0x000FC6DAu);
        wr8(m, obj + 0x06u, 0x09u);
        break;

    default:
        break;
    }

    uint32_t a1 = rd32(m, obj + 0x68u);
    if (a1 != rd32(m, a4 + 0x20u)) {
        wr32(m, a4 + 0x20u, a1);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, a1)));
    }

    int16_t yoff = (int16_t)rd16(m, 0x000FBCEAu + (uint32_t)rd8(m, obj + 0x06u) * 2u);
    if (yoff != 0) {
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 0x10);
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + yoff);
        if ((uint8_t)Terrain_TestPoint(m, tx, ty) == 0) {
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
        } else if ((uint8_t)Terrain_TestPoint(m, tx, (int16_t)(ty - 3)) != 0) {
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u));
        }
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    wr32(m, a4 + 0x24u, rd32(m, obj + 0x10u));
    wr8(m, a4 + 0x28u, rd8(m, obj + 0x1Cu));
    uint32_t sub = rd32(m, obj + 0x68u) + 2u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, sub)));
    sub += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, sub)));
    sub += 2u;
    wr32(m, obj + 0x10u, rd32(m, sub));
    sub += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, sub));
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x10u, rd32(m, a4 + 0x24u));
    wr8(m, obj + 0x1Cu, rd8(m, a4 + 0x28u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr8(m, obj + 0x4Fu, 0u);
    return 0;
}

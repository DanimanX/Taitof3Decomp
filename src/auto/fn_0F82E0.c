/* ROM 0x0F82E0: walking/aiming ground enemy tick (A0 = object).
 * Handles debug-kill check (A5+0x1D58 bit 1, X in [-10, 0x11C]), HP/bomb
 * death (spawning 0x0DC456 child + clearing +0x06 + 0x0CEE74), timer +0x0C,
 * vertical scroll (A5+0x4224/0x4228), 4-state machine (0x0F83D8: enter screen,
 * patrol/terrain-bounce/aim+fire, spawn 0x0D9622 shot, scroll off), then
 * offscreen check (0x0CF85A), script step (0x0D0690), collision (0x0D0DC2),
 * and hit flash (0x0CBE34). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"

#define REC_0F854E 0x000F854Eu
#define REC_0F85AA 0x000F85AAu

/* ROM 0x0D0C8C: difficulty-scaled aim threshold offset from A5+0x87F. */
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
    if (cx < 0x28 || cx > 0x0127 || cy < 0x10 || cy > 0xDF)
        return;

    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    int16_t best_ang[2] = { -1, -1 };
    uint16_t max_dx[2] = { 0u, 0u };
    uint16_t max_dy[2] = { 0u, 0u };

    uint16_t r_min = rd16(m, a1 + 0x08u);
    uint16_t r_max = rd16(m, a1 + 0x0Au);
    uint16_t a_min = rd16(m, a1 + 0x0Cu);
    uint16_t a_max = rd16(m, a1 + 0x0Eu);

    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t sdy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u)
                                + rd16(m, p68 + 2u) - 0x0F);
        uint16_t ady = (uint16_t)(sdy < 0 ? -sdy : sdy);
        if (ady > r_max)
            continue;
        int16_t sdx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u)
                                + rd16(m, p68) - 0x17);
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

int Fn_0F82E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0F82E0: debug kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0F82F8 */
            r->d[0] = 0x00000320u;                            /* 0x0F83C2 */
            Score_AddCurrent(m, 0x00000320u);                 /* 0x0F83C8 -> 0x0D0E30 */
            wr8(m, obj + 0x06u, 0u);                          /* 0x0F83CE */
            /* 0x0F83D2 bra.l 0x0CEF0E */
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
                x >= -0x0A && x <= 0x011C) {
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
                Obj_OrbitDebug(m, obj);
                return 0;
            }
            wr32(m, obj, 0x000CEF44u);
            return 0;
        }
    }

    /* 0x0F8304: HP <= 0 or bomb flag */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        r->d[0] = 0x00000320u;                                /* 0x0F8368 */
        Score_OnHit(m, obj, 0x00000320u);                     /* 0x0F836E */
        goto die_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {            /* 0x0F830C */
        goto die_spawn;
    }

    /* 0x0F8316: countdown timer and Y scroll */
    {
        uint16_t t = rd16(m, obj + 0x0Cu);
        if (t != 0u)
            wr16(m, obj + 0x0Cu, (uint16_t)(t - 1u));
    }
    {
        uint32_t d0 = rd32(m, F3_A5 + 0x4228u);               /* 0x0F8322 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);      /* 0x0F8326 */
        r->d[0] = d0 & 0xFFFF0000u;
    }

    /* 0x0F832A..0x0F8330: jump table 0x0F83D8 */
    switch (rd8(m, obj + 0x06u)) {
    case 1:                                                   /* 0x0F83EC */
        if ((int16_t)rd16(m, obj + 0x3Au) <= 0x0127) {
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x10u);
            wr8(m, obj + 0x06u, 2u);
        }
        /* fall through to 0x0F8402 */
        /* FALLTHROUGH */
    case 4:                                                   /* 0x0F8402 */
        {
            uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
            r->d[0] = d0;
        }
        break;
    case 2:                                                   /* 0x0F840C */
        if (rd16(m, obj + 0x0Cu) == 0u) {
            wr8(m, obj + 0x06u, 4u);
            break;
        }
        if (rd8(m, obj + 0x22u) != 0u) {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
            wr8(m, obj + 0x22u, c);
            if (c == 0u) {
                uint16_t rval = Obj_Next4Way(m);
                r->d[0] = (r->d[0] & 0xFFFF0000u) | rval;
                if ((int8_t)rval >= 0)
                    wr16(m, obj + 0x58u, rd16(m, obj + 0x58u) ^ 0x00B4u);
            }
        }
        {
            uint8_t f7 = rd8(m, obj + 0x07u);
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if ((f7 & 0x02u) != 0u) {
                if (ObjScript_CF1E4(m, obj) < 0)
                    return -1;
            }
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            r->a[1] = REC_0F854E;
            if (ObjScript_CF4B8(m, obj, REC_0F854E) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x22u) == 0u) {
            r->a[1] = REC_0F854E;
            sub_0D07B4(m, obj, REC_0F854E);
            if (rd8(m, obj + 0x23u) != 0u) {
                r->a[1] = REC_0F85AA;
                Obj_StartBehaviour(m, obj, REC_0F85AA);
                wr8(m, obj + 0x23u, 0u);
                wr8(m, obj + 0x06u, 3u);
                break;
            }
        }
        if (rd16(m, obj + 0x58u) != 0u) {
            int16_t x = (int16_t)rd16(m, obj + 0x3Au);
            int16_t y = (int16_t)(rd16(m, obj + 0x3Eu) + 0x10u);
            if (Terrain_TestPoint(m, x, y) != 0)
                wr16(m, obj + 0x58u, 0u);
        } else {
            int16_t x = (int16_t)rd16(m, obj + 0x3Au);
            int16_t y = (int16_t)(rd16(m, obj + 0x3Eu) + 0x10u);
            if (x > 0x0127 || Terrain_TestPoint(m, (int16_t)(x + 0x1F), y) != 0)
                wr16(m, obj + 0x58u, 0x00B4u);
        }
        break;
    case 3:                                                   /* 0x0F84E4 */
        {
            uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
            r->d[0] = d0;
            int16_t f68 = (int16_t)rd16(m, obj + 0x68u);
            if (f68 < 0) {
                uint32_t child = Obj_Spawn(m, obj, 0x000D9622u, 9u, 2u, 0u);
                r->d[0] = child;
                if ((int32_t)child >= 0) {
                    r->a[1] = child;
                    wr32(m, child + 0x14u, rd32(m, obj + 0x3Au));
                    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0018u));
                    wr32(m, child + 0x18u, rd32(m, obj + 0x3Eu));
                    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x000Au));
                }
            } else if (f68 > 0) {
                uint16_t rval = (uint16_t)((Obj_Next4Way(m) & 0x1Fu) + 1u);
                r->d[0] = (r->d[0] & 0xFFFF0000u) | rval;
                wr8(m, obj + 0x22u, (uint8_t)rval);
                wr8(m, obj + 0x06u, 2u);
            }
        }
        break;
    default:
        return -1;
    }

    /* 0x0F8338..0x0F8366 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Obj_FlagOffscreen(m, obj);                                /* 0x0F8344 -> 0x0CF85A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {                /* 0x0F834A */
        wr8(m, obj + 0x06u, 0u);                              /* 0x0F83B8 */
        Obj_StartDying(m, obj);                               /* 0x0F83BC -> 0x0CEE74 */
        return 0;
    }
    Obj_StepScript(m, obj);                                   /* 0x0F8354 -> 0x0D0690 */
    Coll_AddEnemyHazard(m, obj);                              /* 0x0F835A -> 0x0D0DC2 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    Obj_HitFlash(m, obj);                                     /* 0x0F8360 -> 0x0CBE34 */
    return 0;

die_spawn:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u); /* 0x0F8374..0x0F8386 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0018u));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
        wr8(m, obj + 0x06u, 0u);                              /* 0x0F83B8 */
        Obj_StartDying(m, obj);                               /* 0x0F83BC -> 0x0CEE74 */
        return 0;
    }
}

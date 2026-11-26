/* ROM 0x0F9AD4: enemy object tick (A0 = obj).
 * Handles debug on-screen kill (0x0CEF0E), HP <= 0 / bomb-clear death
 * (spawning 0x0DC456), +0x0C timer decrement, scroll Y, state machine (+0x06:
 * 1=enter & run scripts, 2=patrol/aim, 3=scroll/fire 0x0D9992, 4=scroll X),
 * off-screen flag (0x0CF85A), script step (0x0D0690), collision (0x0D0DC2)
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

#define REC_0F9D9E 0x000F9D9Eu /* lea (0x1a4,PC),A1 at 0x0F9BF8, 0x0F9C98, 0x0F9CAA */
#define REC_0F9DFA 0x000F9DFAu /* lea (0x13c,PC),A1 at 0x0F9CBC */

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0CF4B8 wrapper tracking D0/D1 left by sub_0D2D44 in modes 12..15. */
static int cf4b8_regs(const f3mem_t *m, uint32_t obj, uint32_t rec,
                      uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t op = rd8(m, obj + 0x20u);
    if (ObjScript_CF4B8(m, obj, rec) < 0)
        return -1;
    if (op >= 12u && op <= 15u)
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), d0_io, d1_io);
    return 0;
}

/* ROM 0x0C6632 wrapper tracking D0/D1 register state. */
static uint8_t terrain_test_regs(const f3mem_t *m, int16_t x, int16_t y,
                                 uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t hit = (uint8_t)Terrain_TestPoint(m, x, y);
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u);
        return 0u;
    }
    uint32_t d0 = (uint32_t)(((uint16_t)((uint16_t)x + rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x3FFu);
    uint32_t d1 = (uint32_t)(((uint16_t)((uint16_t)y + rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x3Fu);
    d1 = (uint32_t)((int32_t)(int16_t)d1 * 0x280);
    d0 += d1;
    *d1_io = d1;
    *d0_io = (d0 & 0xFFFFFF00u) | (hit ? 0xFFu : 0x00u);
    return hit;
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07B4: aim at nearest live player within bounds/angle window. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint32_t p68 = rd32(m, obj + 0x68u);
    int16_t cx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, p68));
    *d0_io = (*d0_io & 0xFFFF0000u) | (uint16_t)cx;
    if (cx < 0x28 || cx > 0x0127)
        return;
    int16_t cy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, p68 + 2u));
    *d0_io = (*d0_io & 0xFFFF0000u) | (uint16_t)cy;
    if (cy < 0x10 || cy > 0xDF)
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
    if ((int16_t)thr > (int16_t)Rng_Mix1D60(m))
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

int Fn_0F9AD4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    /* 0x0F9AD4: debug on-screen kill */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));         /* 0x0F9AEC */
            d0 = 0x00000320u;                                 /* 0x0F9BB6 */
            Score_AddCurrent(m, d0);                          /* 0x0F9BBC */
            a1 = rd32(m, F3_A5 + 0x1D50u);
            wr8(m, obj + 0x06u, 0u);                          /* 0x0F9BC2 */
            Obj_0CEF0E(m, obj);                               /* 0x0F9BC6 -> 0x0CEF0E */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }
    }

    /* 0x0F9AF8: HP <= 0 or bomb-clear */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        d0 = 0x00000320u;                                     /* 0x0F9B5C */
        Score_OnHit(m, obj, d0);                              /* 0x0F9B62 */
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {     /* 0x0F9B00 */
        if (rd16(m, obj + 0x0Cu) != 0u)                       /* 0x0F9B0A */
            wr16(m, obj + 0x0Cu, (uint16_t)(rd16(m, obj + 0x0Cu) - 1u));

        d0 = rd32(m, F3_A5 + 0x4228u);                        /* 0x0F9B16 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);      /* 0x0F9B1A */

        switch (rd8(m, obj + 0x06u)) {                        /* 0x0F9B24 -> table 0x0F9BCC */
        case 1: {                                             /* 0x0F9BE0 */
            uint8_t f7 = rd8(m, obj + 0x07u);
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if ((f7 & 0x02u) != 0u) {
                if (ObjScript_CF1E4(m, obj) < 0)
                    return -1;
            }
            if (rd8(m, obj + 0x20u) != 0u) {
                a1 = REC_0F9D9E;
                if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)
                    return -1;
            }
            int16_t x3a = (int16_t)rd16(m, obj + 0x3Au);
            if (x3a <= 0x0127) {
                int16_t tx = (int16_t)(x3a + 0x2F);
                int16_t ty = (int16_t)((int16_t)rd16(m, obj + 0x3Eu) + 0x10);
                uint8_t hit = terrain_test_regs(m, tx, ty, &d0, &d1);
                if (hit == 0u && x3a < 0x0028)
                    break;
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            d0 = (d0 & 0xFFFF0000u) | (Rng_Mix1D60(m) & 0x001Fu);
            wr8(m, obj + 0x22u, (uint8_t)d0);
            wr8(m, obj + 0x06u, 2u);
            break;
        }
        case 2: {                                             /* 0x0F9C4E */
            if (rd16(m, obj + 0x0Cu) == 0u) {
                wr8(m, obj + 0x06u, 4u);
                break;
            }
            if (rd8(m, obj + 0x22u) != 0u) {
                uint8_t b = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
                wr8(m, obj + 0x22u, b);
                if (b == 0u) {
                    uint16_t rng = Rng_Mix1D60(m);
                    d0 = (d0 & 0xFFFF0000u) | rng;
                    if ((int8_t)(rng & 0xFFu) >= 0)
                        wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) ^ 0x00B4u));
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
                a1 = REC_0F9D9E;
                if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)
                    return -1;
            }
            if (rd8(m, obj + 0x22u) == 0u) {
                a1 = REC_0F9D9E;
                sub_0D07B4(m, obj, a1, &d0);
                if (rd8(m, obj + 0x23u) != 0u) {
                    a1 = REC_0F9DFA;
                    Obj_SetAnim(m, obj, a1);
                    wr8(m, obj + 0x23u, 0u);
                    wr8(m, obj + 0x06u, 3u);
                    break;
                }
            }
            if (rd16(m, obj + 0x58u) != 0u) {
                int16_t tx = (int16_t)((int16_t)rd16(m, obj + 0x3Au) + 0x10);
                int16_t ty = (int16_t)((int16_t)rd16(m, obj + 0x3Eu) + 0x10);
                uint8_t hit = terrain_test_regs(m, tx, ty, &d0, &d1);
                if (hit != 0u)
                    wr16(m, obj + 0x58u, 0u);
            } else {
                int16_t x3a = (int16_t)rd16(m, obj + 0x3Au);
                if (x3a <= 0x0127) {
                    int16_t tx = (int16_t)(x3a + 0x2F);
                    int16_t ty = (int16_t)((int16_t)rd16(m, obj + 0x3Eu) + 0x10);
                    uint8_t hit = terrain_test_regs(m, tx, ty, &d0, &d1);
                    if (hit == 0u)
                        break;
                }
                wr16(m, obj + 0x58u, 0x00B4u);
            }
            break;
        }
        case 3: {                                             /* 0x0F9D2A */
            d0 = rd32(m, F3_A5 + 0x4224u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
            int16_t w = (int16_t)rd16(m, obj + 0x68u);
            if (w < 0) {
                d0 = Obj_Spawn(m, obj, 0x000D9992u, 9u, 2u, 0u);
                if ((int32_t)d0 >= 0) {
                    a1 = d0;
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au));
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0017u));
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu));
                    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x000Au));
                }
            } else if (w > 0) {
                uint16_t rng = Rng_Mix1D60(m);
                d0 = (d0 & 0xFFFF0000u) | ((rng & 0x000Fu) + 1u);
                wr8(m, obj + 0x22u, (uint8_t)d0);
                wr8(m, obj + 0x06u, 2u);
            }
            break;
        }
        case 4:                                               /* 0x0F9D94 */
            d0 = rd32(m, F3_A5 + 0x4224u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
            break;
        default:
            break;
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));           /* 0x0F9B2C */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));           /* 0x0F9B32 */
        Obj_FlagOffscreen(m, obj);                            /* 0x0F9B38 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {            /* 0x0F9B3E */
            wr8(m, obj + 0x06u, 0u);                          /* 0x0F9BAC */
            Obj_StartDying(m, obj);                           /* 0x0F9BB0 */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }

        {
            uint16_t old_cnt = rd16(m, obj + 0x4Cu);
            Obj_StepScript(m, obj);                           /* 0x0F9B48 */
            if (old_cnt == 1u) {
                uint32_t scr = rd32(m, obj + 0x48u);
                uint16_t w = rd16(m, scr);
                if ((int16_t)w < 0)
                    d0 = (d0 & 0xFFFF0000u) | w;
                else
                    d0 = (d0 & 0xFFFF0000u) | rd16(m, scr + 2u);
            }
        }
        Coll_AddEnemyHazard(m, obj);                          /* 0x0F9B4E */
        a1 = rd32(m, F3_A5 + 0x0906u);
        Obj_HitFlash(m, obj);                                 /* 0x0F9B54 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    /* 0x0F9B68: death spawn */
    d0 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));            /* 0x0F9B8C */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0020u)); /* 0x0F9B92 */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));            /* 0x0F9B98 */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0018u)); /* 0x0F9B9E */
        wr16(m, a1 + 0x58u, 0u);                              /* 0x0F9BA4 */
        wr16(m, a1 + 0x60u, 0u);                              /* 0x0F9BA8 */
    }
    wr8(m, obj + 0x06u, 0u);                                  /* 0x0F9BAC */
    Obj_StartDying(m, obj);                                   /* 0x0F9BB0 */
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}

/* ROM 0x10C548: aiming turret/cannon enemy tick (A0 = object).
 * Handles bomb-clear (0x10C636 -> +0x12C score, 0x65710 sprite, 0x0CBB2C),
 * death/pause (0x10C5D2/0x10C5DE -> +0x12C score on hit, spawn 0x0DC456,
 * 0x0CEE74), scroll integration, offscreen check 0x0CF85A, state dispatch
 * 0x10C65C (1: arm on screen + transition to 2, 2: aim 0x0D07B4(0x10C734) ->
 * start anim 0x10C784 + state 3, 3: cooldown +0x23 -> aim 0x0D07B4(0x10C094)
 * with offset 0x10C7FC and spawn 0x10C242), and tail script/hazard/flash/draw. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
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

int Fn_10C548(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x10C548: bomb-clear check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x10C560 */
            wr8(m, obj + 0x06u, 0u);                        /* 0x10C636 */
            Score_AddCurrent(m, 0x012Cu);                   /* 0x10C63A..0x10C640 */
            wr32(m, obj + 0x10u, 0x00065710u);              /* 0x10C646 */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 3u)); /* 0x10C64E */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u)); /* 0x10C652 */
            Obj_OrbitDebug(m, obj);                         /* 0x10C656 */
            return 0;
        }
    }

    /* 0x10C56C..0x10C57A */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x012Cu);                       /* 0x10C5D2..0x10C5D8 */
        goto spawn_die;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto spawn_die;

    /* 0x10C57E..0x10C594 */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    Obj_FlagOffscreen(m, obj);                              /* 0x10C58E */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {              /* 0x10C594..0x10C59A */
        wr8(m, obj + 0x06u, 0u);                            /* 0x10C62C */
        Obj_StartDying(m, obj);                             /* 0x10C630 */
        return 0;
    }

    /* 0x10C59E..0x10C5B0 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 1u:
        Obj_ArmOnScreen(m, obj);                            /* 0x10C66C */
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {          /* 0x10C672..0x10C678 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x10C67C */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x10C682 */
            wr8(m, obj + 0x06u, 2u);                        /* 0x10C688 */
        }
        break;
    case 2u:
        sub_0D07B4(m, obj, 0x0010C734u);                    /* 0x10C694 */
        if (rd8(m, obj + 0x23u) != 0u) {                    /* 0x10C69A..0x10C69E */
            Obj_SetAnim(m, obj, 0x0010C784u);               /* 0x10C6A6 */
            wr8(m, obj + 0x23u, 0u);                        /* 0x10C6AC */
            wr8(m, obj + 0x06u, 3u);                        /* 0x10C6B0 */
        }
        break;
    case 3u:
        if ((int16_t)rd16(m, obj + 0x68u) < 0) {            /* 0x10C6B8..0x10C6BC */
            uint8_t c = rd8(m, obj + 0x23u);
            if (c != 0u) {
                c = (uint8_t)(c - 1u);
                wr8(m, obj + 0x23u, c);                     /* 0x10C6C8 */
                if (c != 0u)
                    break;
            }
            {
                uint32_t saved_68 = rd32(m, obj + 0x68u);   /* 0x10C6D0 */
                wr32(m, obj + 0x68u, 0x0010C7FCu);          /* 0x10C6D4 */
                sub_0D07B4(m, obj, 0x0010C094u);            /* 0x10C6E0 */
                wr32(m, obj + 0x68u, saved_68);             /* 0x10C6E6 */
                if (rd8(m, obj + 0x23u) != 0u) {            /* 0x10C6EA */
                    uint32_t child = Obj_Spawn(m, obj, 0x0010C242u, 3u, 2u, 0u);
                    if ((int32_t)child >= 0) {
                        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0019u));
                        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0007u));
                        wr16(m, child + 0x58u, 0x0120u);
                    }
                }
            }
        }
        break;
    default:
        return -1;
    }

    /* 0x10C5B8..0x10C5D0 */
    Obj_StepScript(m, obj);
    Obj_0D0DC2(m, obj);
    Obj_HitFlash(m, obj);
    draw_sprite_regs(m, r);
    return 0;

spawn_die:
    wr8(m, obj + 0x06u, 0u);                                /* 0x10C5DE */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0018u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0007u));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
    }
    Obj_StartDying(m, obj);                                 /* 0x10C626 */
    return 0;
}

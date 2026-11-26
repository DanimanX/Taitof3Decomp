/* ROM 0x0D9A24: enemy object tick (A0 = obj).
 * Handles debug on-screen kill (0x0CBB2C), HP <= 0 / bomb-clear death
 * (spawning 0x0DBDA2), +0x23 countdown, scroll-adjusted movement (+0x50/+0x54),
 * state 0 player Y-alignment attack (0x0D0678 + 0x0D2D44), bounds & terrain
 * check (0x0C6632), script step (0x0D0690), hazard (0x0D0DCC), sprite (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

#define REC_0D9902 0x000D9902u /* lea (-0x2a6,PC),A1 at 0x0D9BA6 */
#define REC_0D994A 0x000D994Au /* lea (-0x2a8,PC),A1 at 0x0D9BF0 */
#define TBL_0D9C38 0x000D9C38u /* lea (0x182,PC,D2w*4),A1 at 0x0D9AB4 */

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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0D9A24(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    /* 0x0D9A24: debug on-screen kill */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_OrbitDebug(m, obj);                           /* 0x0D9A3C -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0D9A42: HP <= 0 or bomb-clear */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        uint8_t st = rd8(m, obj + 0x06u);                     /* 0x0D9AE2 */
        d0 = (d0 & 0xFFFFFF00u) | st;
        if (st == 0u)
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
        else if (st == 2u)
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x004Fu));
        d0 = Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);      /* 0x0D9B02 */
        Obj_StartDying(m, obj);                               /* 0x0D9B1E -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    if (rd8(m, obj + 0x23u) != 0u)                            /* 0x0D9A54 */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));

    d0 = rd32(m, obj + 0x50u);                                /* 0x0D9A60 */
    if (rd8(m, obj + 0x06u) == 0u)                            /* 0x0D9A64 */
        d0 -= rd32(m, F3_A5 + 0x4224u);                       /* 0x0D9A6C */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);          /* 0x0D9A70 */

    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);     /* 0x0D9A74..0x0D9A78 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);          /* 0x0D9A7C */

    if (rd8(m, obj + 0x06u) == 0u) {                          /* 0x0D9A86 -> 0x0D9B30 */
        if (rd8(m, obj + 0x23u) == 0u &&
            (int16_t)rd16(m, obj + 0x14u) <= 0x0120) {
            uint32_t target = 0u;
            a1 = 0x0041B190u;
            if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
                int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, a1 + 0x18u) - 6);
                if (dy < 0) dy = (int16_t)(-dy);
                d0 = (d0 & 0xFFFF0000u) | (uint16_t)dy;
                if ((uint16_t)dy <= 3u)
                    target = a1;
            }
            if (target == 0u) {
                a1 = 0x0041B21Au;
                if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
                    int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, a1 + 0x18u) - 6);
                    if (dy < 0) dy = (int16_t)(-dy);
                    d0 = (d0 & 0xFFFF0000u) | (uint16_t)dy;
                    if ((uint16_t)dy <= 3u)
                        target = a1;
                }
            }
            if (target != 0u) {                               /* 0x0D9B92 */
                wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 3u));
                int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, target + 0x14u) + 1);
                if (dx >= 0) {                                /* 0x0D9BA6 */
                    a1 = REC_0D9902;
                    Obj_SetAnim(m, obj, a1);
                    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Cu));
                    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
                    wr16(m, obj + 0x2Cu, 8u);
                    wr16(m, obj + 0x2Eu, 0x0Fu);
                    wr16(m, obj + 0x30u, 4u);
                    wr16(m, obj + 0x32u, 9u);
                    sub_0D2D44(m, 0x00B4u, rd16(m, obj + 0x60u), &d0, &d1);
                    wr32(m, obj + 0x50u, d0);
                    wr32(m, obj + 0x54u, d1);
                    wr8(m, obj + 0x06u, 1u);
                } else {                                      /* 0x0D9BF0 */
                    a1 = REC_0D994A;
                    Obj_SetAnim(m, obj, a1);
                    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x003Bu));
                    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
                    wr16(m, obj + 0x2Cu, 0x0050u);
                    wr16(m, obj + 0x2Eu, 0x0057u);
                    wr16(m, obj + 0x30u, 4u);
                    wr16(m, obj + 0x32u, 9u);
                    sub_0D2D44(m, 0u, rd16(m, obj + 0x60u), &d0, &d1);
                    wr32(m, obj + 0x50u, d0);
                    wr32(m, obj + 0x54u, d1);
                    wr8(m, obj + 0x06u, 2u);
                }
            }
        }
    }

    /* 0x0D9A8E: offscreen & terrain check */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x < -0x70 || x > 0x01AF) {
            Obj_StartDying(m, obj);                           /* 0x0CEE74 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    {
        uint16_t st = rd8(m, obj + 0x06u);
        a1 = TBL_0D9C38 + (uint32_t)st * 4u;
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, a1));
        a1 += 2u;
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, a1));
        uint8_t hit = (uint8_t)Terrain_TestPoint(m, tx, ty);  /* 0x0D9AC0 -> 0x0C6632 */
        d0 = (d0 & 0xFFFFFF00u) | hit;
        if (hit != 0u) {
            Obj_StartDying(m, obj);                           /* 0x0CEE74 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    Obj_StepScript(m, obj);                                   /* 0x0D9ACE -> 0x0D0690 */
    Coll_AddHazard(m, obj);                                   /* 0x0D9AD4 -> 0x0D0DCC */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                                   /* 0x0D9ADA -> 0x001282 */
    return 0;
}

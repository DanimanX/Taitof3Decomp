/* ROM 0x0E4F0A: carried turret/wing gun tick (A0 = obj, +0x0C = carrier).
 * Dies if carrier state == 0 or HP <= 0 (setting carrier state = 3 and
 * spawning 0x0DC456). Otherwise tracks carrier position + frame offset table
 * 0x0E5172[carrier+0x23], checks offscreen (0x0CF85A), runs state machine
 * (+0x06: 0=arm, 1=aim/track, 2=fire 0x0D9224, 3=cooldown), registers
 * collision (0x0D0DC2), flashes hit colour (0x0CBE34), and draws both self
 * and carrier (leaving A0 = carrier). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

#define REC_0E513A 0x000E513Au /* lea (0x108,PC),A1 at 0x0E5030 */
#define TBL_0E5172 0x000E5172u /* lea (0x240,PC,D0w*4),A1 at 0x0E4F30 */
#define TBL_0E5182 0x000E5182u /* lea (0x140,PC),A1 at 0x0E5040 */
#define TBL_0E5222 0x000E5222u /* lea (0x1c2,PC),A1 at 0x0E505E */

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

/* ROM 0x0D0BBC: turn obj+0x08 toward obj+0x42. */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d1w = rd16(m, obj + 0x42u);
    int16_t diff = (int16_t)(d1w - rd16(m, obj + 0x08u));
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);
    if ((uint16_t)diff > 0x00B4u) {
        diff = (int16_t)(0x0168 - diff);
        if ((uint16_t)diff <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, d1w);
            return;
        }
        int32_t v = (int32_t)rd32(m, obj + 0x08u) - (int32_t)rd32(m, a1 + 0x04u);
        wr32(m, obj + 0x08u, (uint32_t)v);
        if (v < 0)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
    } else {
        if ((uint16_t)diff <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, d1w);
            return;
        }
        uint32_t v = rd32(m, obj + 0x08u) + rd32(m, a1 + 0x04u);
        wr32(m, obj + 0x08u, v);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
    }
}

/* ROM 0x0D0090: install animation record from base + table 0x0D00B4[d0]. */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    int16_t off = (int16_t)rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)d0w * 2u);
    uint32_t ptr = rd32(m, (uint32_t)((int32_t)a1 + (int32_t)off));
    wr32(m, obj + 0x48u, ptr);
    wr16(m, obj + 0x4Cu, rd16(m, ptr));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, ptr + 2u));
    wr32(m, obj + 0x10u, rd32(m, ptr + 4u));
    wr32(m, obj + 0x68u, rd32(m, ptr + 8u));
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

int Fn_0E4F0A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x0Cu);                       /* 0x0E4F0A */

    if (rd8(m, a1 + 0x06u) != 0u) {                           /* 0x0E4F0E */
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {             /* 0x0E4F16 */
            wr8(m, a1 + 0x06u, 3u);                           /* 0x0E4F8C */
            Score_OnHit(m, obj, 0x0000012Cu);                 /* 0x0E4F92..0x0E4F98 */
        } else {
            wr32(m, obj + 0x3Au, rd32(m, a1 + 0x14u));        /* 0x0E4F1E */
            wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));        /* 0x0E4F24 */
            uint16_t idx = rd8(m, a1 + 0x23u);                /* 0x0E4F2C */
            uint32_t off_p = TBL_0E5172 + (uint32_t)idx * 4u; /* 0x0E4F30 */
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, off_p)));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, off_p + 2u)));

            Obj_FlagOffscreen(m, obj);                        /* 0x0E4F44 */
            if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {        /* 0x0E4F4A */
                a1 = rd32(m, obj + 0x0Cu);
                wr8(m, a1 + 0x06u, 3u);
                Obj_StartDying(m, obj);
                r->a[1] = a1;
                return 0;
            }

            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));       /* 0x0E4F54 */
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));       /* 0x0E4F5A */

            switch (rd8(m, obj + 0x06u)) {                    /* 0x0E4F66 -> table 0x0E4FFA */
            case 0:                                           /* 0x0E500A */
                Obj_ArmOnScreen(m, obj);
                if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
                    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
                    wr8(m, obj + 0x06u, 1u);
                }
                break;
            case 1:                                           /* 0x0E5028 */
                if (rd8(m, obj + 0x23u) == 0u) {
                    sub_0D07B4(m, obj, REC_0E513A);
                    sub_0D0BBC(m, obj, REC_0E513A);
                    Obj_0CFA7C(m, obj, TBL_0E5182, rd16(m, obj + 0x08u));
                }
                if (rd8(m, obj + 0x23u) != 0u) {
                    uint8_t b = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                    wr8(m, obj + 0x23u, b);
                    if (b == 0u) {
                        sub_0D0090(m, obj, TBL_0E5222, rd16(m, obj + 0x08u));
                        wr8(m, obj + 0x06u, 2u);
                    }
                }
                break;
            case 2:                                           /* 0x0E5074 */
                Obj_StepScript(m, obj);
                if (rd16(m, obj + 0x4Cu) != 0u) {
                    int16_t d1w = (int16_t)rd16(m, obj + 0x68u);
                    if (d1w < 0) {
                        wr8(m, rd32(m, obj + 0x0Cu) + 0x06u, 1u);
                        break;
                    }
                    uint32_t shot = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
                    if ((int32_t)shot >= 0) {
                        wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                        wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0035u));
                        wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                        wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x0025u));
                        wr16(m, shot + 0x58u, (uint16_t)d1w);
                        wr16(m, shot + 0x10u, 0x001Bu);
                        wr8(m, rd32(m, obj + 0x0Cu) + 0x06u, 2u);
                        break;
                    }
                }
                Obj_0CFA7C(m, obj, TBL_0E5182, rd16(m, obj + 0x08u));
                wr8(m, obj + 0x23u, 0x0Au);
                wr8(m, obj + 0x06u, 3u);
                wr8(m, rd32(m, obj + 0x0Cu) + 0x06u, 1u);
                break;
            case 3: {                                         /* 0x0E5102 */
                uint8_t b = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                wr8(m, obj + 0x23u, b);
                if (b == 0u) {
                    wr8(m, obj + 0x06u, 1u);
                } else {
                    sub_0D07B4(m, obj, REC_0E513A);
                    wr8(m, obj + 0x23u, b);
                    sub_0D0BBC(m, obj, REC_0E513A);
                    Obj_0CFA7C(m, obj, TBL_0E5182, rd16(m, obj + 0x08u));
                }
                break;
            }
            default:
                break;
            }

            Coll_AddEnemyHazard(m, obj);                      /* 0x0E4F6E */
            Obj_HitFlash(m, obj);                             /* 0x0E4F74 */
            Obj_DrawSprite(m, obj);                           /* 0x0E4F7A */
            r->a[0] = rd32(m, obj + 0x0Cu);                   /* 0x0E4F80 */
            draw_sprite_regs(m, r);                           /* 0x0E4F84 */
            return 0;
        }
    }

    /* 0x0E4F9E: death spawn */
    uint32_t d0 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
    r->d[0] = d0;
    if ((int32_t)d0 >= 0) {
        r->a[1] = d0;
        wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x002Eu));
        wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x0025u));
        wr16(m, d0 + 0x58u, 0u);
        wr16(m, d0 + 0x60u, 0u);
    }
    Obj_StartDying(m, obj);
    return 0;
}

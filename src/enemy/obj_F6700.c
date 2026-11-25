#include "obj_F6700.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CF82C: arm script when Y enters [-64, 247]. */
static void sub_0CF82C(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
        return;
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (y < -0x40 || y > 0x00F7)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((rd8(m, obj + 0x07u) & ~0x80u) | 0x02u));
}

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

/* ROM 0x0F6700 */
int Obj_F6700_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0F6700 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        /* 0x0F6D16 */
        wr8(m, obj + 0x06u, 0u);
        Obj_SetFrame(m, obj, 0x000F738Au);
        Score_AddCurrent(m, 0x000007D0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        Obj_OrbitDebug(m, obj);
        return 0;
    }

    if (rd8(m, obj + 0x06u) == 9u)
        goto lab_0f699c;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        if (rd8(m, obj + 0x06u) >= 4u) {
            /* 0x0F6C30 */
            Score_OnHit(m, obj, 0x000007D0u);
            goto lab_0f6c3c;
        }
        /* 0x0F6726 */
        Sound_PlayHooked(m, 0x0016u);
        Obj_SetScript(m, obj, 0x000F7114u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
        wr32(m, obj + 0x24u, 0x001E0000u);
        wr8(m, obj + 0x23u, 2u);
        wr8(m, obj + 0x06u, 4u);
    }

    /* 0x0F6764 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_0f6c3c;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
    }

    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x000F706Au) < 0)
            return -1;
    }

    if ((rd8(m, obj + 0x4Eu) & 0x40u) != 0u)
        sub_0CF82C(m, obj);
    else
        Obj_ArmOnScreen(m, obj);

    /* 0x0F67B2 */
    if ((int16_t)rd16(m, obj + 0x3Au) < -0x78 || (int16_t)rd16(m, obj + 0x3Eu) > 0x0127)
        goto lab_0f6d0c;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    if (rd8(m, obj + 0x06u) < 4u) {
        uint32_t d0 = 0, d1 = 0;
        Obj_0CF898(m, obj, &d0, &d1);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
    }

    /* 0x0F67E6: jump table 0x0F6D3C */
    switch (rd8(m, obj + 0x06u)) {
    case 1: {
        if (rd8(m, obj + 0x23u) == 0u) {
            sub_0D07B4(m, obj, 0x000F706Au);
            sub_0D0BBC(m, obj, 0x000F706Au);
            Obj_0CFA7C(m, obj, 0x000F7168u, rd16(m, obj + 0x08u));
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c != 0u)
            break;
        sub_0D0090(m, obj, 0x000F7208u, rd16(m, obj + 0x08u));
        wr8(m, obj + 0x06u, 2u);
        uint32_t a1 = Obj_Spawn(m, obj, 0x000F7398u, 9u, 2u, 0u);
        if ((int32_t)a1 >= 0)
            wr16(m, a1 + 0x10u, rd16(m, obj + 0x6Au));
        break;
    }
    case 2: {
        Obj_AnimStep(m, obj);
        if (rd16(m, obj + 0x4Cu) != 0u) {
            uint16_t d1 = rd16(m, obj + 0x68u);
            if ((int16_t)d1 >= 0) {
                uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
                if ((int32_t)a1 >= 0) {
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0021u));
                    wr16(m, a1 + 0x58u, d1);
                    wr16(m, a1 + 0x10u, 0x0010u);
                }
            }
        } else {
            Obj_0CFA7C(m, obj, 0x000F7168u, rd16(m, obj + 0x08u));
            wr8(m, obj + 0x23u, 0x0Au);
            wr8(m, obj + 0x06u, 3u);
        }
        break;
    }
    case 3: {
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c == 0u) {
            wr8(m, obj + 0x06u, 1u);
        } else {
            sub_0D07B4(m, obj, 0x000F706Au);
            wr8(m, obj + 0x23u, c);
            sub_0D0BBC(m, obj, 0x000F706Au);
            Obj_0CFA7C(m, obj, 0x000F7168u, rd16(m, obj + 0x08u));
        }
        break;
    }
    case 4:
    case 5:
    case 6: {
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c == 0u) {
            wr8(m, obj + 0x23u, 2u);
            wr8(m, obj + 0x06u, (uint8_t)(rd8(m, obj + 0x06u) + 1u));
        }
        break;
    }
    case 7:
        Obj_SetFrame(m, obj, 0x000F738Au);
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 5u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));
        wr8(m, obj + 0x06u, 8u);
        break;
    case 8:
        break;
    default:
        return -1;
    }

    /* 0x0F67F4 */
    if (rd8(m, obj + 0x06u) >= 4u) {
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 0x18);
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + 0x20);
        if ((uint8_t)Terrain_TestPoint(m, tx, ty) != 0u)
            goto lab_0f6c3c;
    }

    /* 0x0F681A */
    Coll_AddEnemyHazard(m, obj);
    Obj_HitFlash(m, obj);

    {
        uint32_t cur = rd32(m, obj + 0x54u);
        if (rd32(m, cur + 0x10u) > 2u && rd8(m, cur + 0x06u) != 0u) {
            Obj_DrawSprite(m, cur);
            cur = rd32(m, cur + 0x54u);
            Obj_DrawSprite(m, cur);
            cur = rd32(m, cur + 0x54u);
            Obj_DrawSprite(m, cur);
        }

        /* 0x0F6858 */
        cur = rd32(m, cur + 0x08u);
        if (rd8(m, cur + 0x06u) == 8u || rd8(m, cur + 0x06u) < 4u) {
            /* 0x0F6870 */
            cur = rd32(m, cur + 0x54u);
            cur = rd32(m, cur + 0x54u);
            cur = rd32(m, cur + 0x54u);
            cur = rd32(m, cur + 0x54u);
            if (rd32(m, cur + 0x10u) != 0u && rd8(m, cur + 0x06u) != 0u) {
                uint32_t a1 = rd32(m, cur + 0x08u);
                wr32(m, cur + 0x14u, rd32(m, a1 + 0x14u));
                wr16(m, cur + 0x14u, (uint16_t)(rd16(m, cur + 0x14u) + 0x5Cu));
                wr32(m, cur + 0x18u, rd32(m, a1 + 0x18u));
                wr16(m, cur + 0x18u, (uint16_t)(rd16(m, cur + 0x18u) + 0x1Cu));
                Obj_DrawSprite(m, cur);
            }
            cur = rd32(m, cur + 0x08u);
            Obj_DrawSprite(m, cur);
        } else {
            /* 0x0F68C0 */
            static const uint32_t k_frames[7] = {
                0x000F7344u, 0x000F734Eu, 0x000F7358u, 0x000F7362u,
                0x000F736Cu, 0x000F7376u, 0x000F7380u
            };
            uint16_t dy = (uint16_t)(rd8(m, cur + 0x06u) - 4u);
            for (int i = 0; i < 7; i++) {
                Obj_SetFrame(m, cur, k_frames[i]);
                Obj_DrawSprite(m, cur);
                if (i < 6) {
                    wr16(m, cur + 0x14u, (uint16_t)(rd16(m, cur + 0x14u) + 0x10u));
                    wr16(m, cur + 0x18u, (uint16_t)(rd16(m, cur + 0x18u) - dy));
                }
            }
            wr32(m, cur + 0x14u, rd32(m, cur + 0x3Au));
            wr32(m, cur + 0x18u, rd32(m, cur + 0x3Eu));
        }

        /* 0x0F698E */
        if ((rd8(m, cur + 0x4Eu) & 0x3Fu) == 0u)
            return 0;
        obj = cur;
        goto lab_0f69b2;
    }

lab_0f6c3c:
    {
        uint32_t d1 = rd32(m, obj + 0x14u) + 0x00200000u;
        uint32_t d2 = rd32(m, obj + 0x18u) + 0x00180000u;
        uint16_t d3w = rd16(m, obj + 0x58u);
        uint16_t d4w = rd16(m, obj + 0x60u);
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC5B6u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, d1);
            wr32(m, a1 + 0x18u, d2);
            wr16(m, a1 + 0x58u, d3w);
            wr16(m, a1 + 0x60u, d4w);
            a1 = Obj_Spawn(m, obj, 0x000DC49Au, 3u, 2u, 0u);
            if ((int32_t)a1 >= 0) {
                d1 += 0x00140000u;
                wr32(m, a1 + 0x14u, d1);
                d2 -= 0x00040000u;
                wr32(m, a1 + 0x18u, d2);
                wr16(m, a1 + 0x58u, d3w);
                wr16(m, a1 + 0x60u, d4w);
                a1 = Obj_Spawn(m, obj, 0x000DC49Au, 3u, 2u, 0u);
                if ((int32_t)a1 >= 0) {
                    d1 += 0x00140000u;
                    wr32(m, a1 + 0x14u, d1);
                    d2 -= 0x00040000u;
                    wr32(m, a1 + 0x18u, d2);
                    wr16(m, a1 + 0x58u, d3w);
                    wr16(m, a1 + 0x60u, d4w);
                }
            }
        }
    }

lab_0f6d0c:
    wr8(m, obj + 0x06u, 9u);

lab_0f699c:
    if ((rd8(m, obj + 0x4Eu) & 0x3Fu) == 0u) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

lab_0f69b2:
    {
        uint32_t cur = obj;
        for (int i = 0; i < 5; i++)
            cur = rd32(m, cur + 0x54u);
        if (rd32(m, cur + 0x10u) <= 3u)
            return 0;

        for (int i = 0; i < 4; i++) {
            if (i > 0)
                cur = rd32(m, cur + 0x54u);
            if (rd8(m, cur + 0x06u) == 0u)
                continue;
            if (rd8(m, cur + 0x06u) < 7u) {
                uint32_t a1 = rd32(m, cur + 0x08u);
                wr32(m, cur + 0x14u, rd32(m, a1 + 0x14u));
                wr32(m, cur + 0x18u, rd32(m, a1 + 0x18u));
                uint16_t d0 = rd16(m, cur + 0x0Cu);
                uint32_t a2 = rd32(m, 0x000F6EFAu + (uint32_t)(int16_t)d0 * 4u);
                a2 += (uint32_t)rd8(m, a1 + 0x06u) * 4u;
                wr16(m, cur + 0x14u, (uint16_t)(rd16(m, cur + 0x14u) + rd16(m, a2)));
                wr16(m, cur + 0x18u, (uint16_t)(rd16(m, cur + 0x18u) + rd16(m, a2 + 2u)));
            }
            Obj_DrawSprite(m, cur);
            wr32(m, cur + 0x3Au, rd32(m, cur + 0x14u));
            wr32(m, cur + 0x3Eu, rd32(m, cur + 0x18u));
            uint32_t tbl = (rd8(m, cur + 0x4Fu) != 0u) ? 0x000F700Au : 0x000F6FAAu;
            uint32_t a1 = tbl + (uint32_t)rd8(m, cur + 0x06u) * 8u;
            wr16(m, cur + 0x14u, (uint16_t)(rd16(m, cur + 0x14u) + rd16(m, a1)));
            wr16(m, cur + 0x18u, (uint16_t)(rd16(m, cur + 0x18u) + rd16(m, a1 + 2u)));
            wr32(m, cur + 0x10u, rd32(m, a1 + 4u));
            Obj_DrawSprite(m, cur);
            wr32(m, cur + 0x14u, rd32(m, cur + 0x3Au));
            wr32(m, cur + 0x18u, rd32(m, cur + 0x3Eu));
        }
    }
    return 0;
}

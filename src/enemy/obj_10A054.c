#include "obj_10A054.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[angle.w] * scale.w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
}

/* ROM 0x10A054 */
int Obj_10A054_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x00000190u);
        goto lab_10a1f6;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_10a1f6;

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    {
        int16_t d0 = -0x01F4;
        int16_t d1 = -0x01F4;
        if ((int16_t)rd16(m, 0x0041B190u + 0x4Cu) >= 0)
            d0 = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x0041B190u + 0x18u) + 2);
        if ((int16_t)rd16(m, 0x0041B21Au + 0x4Cu) >= 0)
            d1 = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x0041B21Au + 0x18u) + 2);

        uint16_t ad0 = (uint16_t)(d0 < 0 ? -d0 : d0);
        uint16_t ad1 = (uint16_t)(d1 < 0 ? -d1 : d1);
        if (ad1 < ad0)
            d0 = d1;

        uint16_t target_ang = (d0 < 0) ? 0x010Eu : 0x005Au;
        if (rd16(m, obj + 0x58u) == target_ang) {
            wr32(m, obj + 0x60u, rd32(m, obj + 0x60u) + 0x0000EA60u);
            if ((int16_t)rd16(m, obj + 0x60u) > 0x0019)
                wr16(m, obj + 0x60u, 0x0019u);
        } else {
            int32_t spd = (int32_t)(rd32(m, obj + 0x60u) - 0x000105B8u);
            wr32(m, obj + 0x60u, (uint32_t)spd);
            if (spd < 0) {
                wr16(m, obj + 0x60u, 0u);
                int16_t ang = (int16_t)(rd16(m, obj + 0x58u) - 0x00B4);
                if (ang < 0)
                    ang = (int16_t)(ang + 0x0168);
                wr16(m, obj + 0x58u, (uint16_t)ang);
            }
        }
    }

    {
        uint32_t vx, vy;
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &vx, &vy);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00010000u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);

        if ((int16_t)rd16(m, obj + 0x14u) < -0x20) {
            uint32_t a1 = rd32(m, obj + 0x08u);
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
            wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
            Obj_StartDying(m, obj);
            return 0;
        }

        uint16_t dy_s = (uint16_t)(0u - (uint16_t)(vy >> 16));
        uint16_t spr_idx = Angle360(m, 1u, dy_s);
        Obj_0CFA7C(m, obj, 0x0010A33Au, spr_idx);
        Coll_AddEnemyHazard(m, obj);
        Obj_HitFlash(m, obj);
        Obj_DrawSprite(m, obj);
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x10A2A4 */
            Score_AddCurrent(m, 0x00000190u);
            uint32_t a1 = rd32(m, obj + 0x08u);
            wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
            if (rd8(m, a1 + 0x22u) != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Score_AddCurrent(m, 0x00000FA0u);
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
            Obj_DieWithBonusDbg(m, obj, 0u);
            return 0;
        }
    }

    if (rd8(m, obj + 0x23u) != 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) != 0u)
            return 0;
    }

    Obj_AimAtPlayers(m, obj, 0x0010A302u);
    if (rd8(m, obj + 0x23u) != 0u) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x000Fu));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0011u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));
            wr16(m, a1 + 0x10u, 0u);
        }
    }
    return 0;

lab_10a1f6:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC56Eu, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x000Fu));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0011u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
        a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        if (rd8(m, a1 + 0x22u) != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
            Obj_DieWithBonusChk(m, obj, 0u);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        Score_OnHit(m, obj, 0x00000FA0u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
        Obj_DieWithBonus(m, obj, 0u);
        return 0;
    }
}

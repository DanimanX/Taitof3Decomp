#include "obj_ED2D8.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0D06E0 */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, 0x00B4u);

    uint32_t p1 = 0x0041B190u;
    int16_t d4 = -1;
    int16_t d2 = 0, d3 = 0;
    if ((int16_t)rd16(m, p1 + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        int16_t raw_dx = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, p1 + 0x14u) +
                                   (int16_t)rd16(m, a2) - 0x17);
        d2 = (raw_dx < 0) ? (int16_t)-raw_dx : raw_dx;
        int16_t raw_dy = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, p1 + 0x18u) +
                                   (int16_t)rd16(m, a2 + 2u) - 0x0F);
        d3 = (raw_dy < 0) ? (int16_t)-raw_dy : raw_dy;
        d4 = (int16_t)Angle360(m, (uint16_t)raw_dx, (uint16_t)raw_dy);
    }

    uint32_t p2 = 0x0041B21Au;
    int16_t d7 = -1;
    int16_t d5 = 0, d6 = 0;
    if ((int16_t)rd16(m, p2 + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        int16_t raw_dx = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, p2 + 0x14u) +
                                   (int16_t)rd16(m, a2) - 0x17);
        d5 = (raw_dx < 0) ? (int16_t)-raw_dx : raw_dx;
        int16_t raw_dy = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, p2 + 0x18u) +
                                   (int16_t)rd16(m, a2 + 2u) - 0x0F);
        d6 = (raw_dy < 0) ? (int16_t)-raw_dy : raw_dy;
        d7 = (int16_t)Angle360(m, (uint16_t)raw_dx, (uint16_t)raw_dy);
    }

    if (d4 < 0 && d7 < 0)
        return;

    if (d4 >= 0 && d7 >= 0) {
        uint16_t dist1 = ((uint16_t)d2 > (uint16_t)d3) ? (uint16_t)d2 : (uint16_t)d3;
        uint16_t dist2 = ((uint16_t)d5 > (uint16_t)d6) ? (uint16_t)d5 : (uint16_t)d6;
        wr16(m, obj + 0x42u, (dist2 < dist1) ? (uint16_t)d7 : (uint16_t)d4);
        return;
    }

    wr16(m, obj + 0x42u, (d4 >= 0) ? (uint16_t)d4 : (uint16_t)d7);
}

/* ROM 0x0ED496 */
static void state_0ED496(const f3mem_t *m, uint32_t obj)
{
    uint8_t t = rd8(m, obj + 0x23u);
    int fire = 1;
    if (t != 0u) {
        t = (uint8_t)(t - 1u);
        wr8(m, obj + 0x23u, t);
        if (t != 0u)
            fire = 0;
    }

    if (fire) {
        /* 0ED4A8 + 0xC2 = 0x0ED56A */
        Obj_AimAtPlayers(m, obj, 0x000ED56Au);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint32_t shot = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)shot >= 0) {
                wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0Cu));
                wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x0Cu));
                wr16(m, shot + 0x58u, rd16(m, obj + 0x42u));
                wr16(m, shot + 0x10u, 0u);
            }
        }
    }

    if (rd8(m, obj + 0x20u) == 0u) {
        wr8(m, obj + 0x06u, 1u);
        sub_0D06E0(m, obj);
        if (rd16(m, obj + 0x42u) < 0x00B4u) {
            Obj_SetScript(m, obj, 0x000ED5C0u);
            Obj_SetAnim(m, obj, 0x000ED650u);
        } else {
            Obj_SetScript(m, obj, 0x000ED5DEu);
            Obj_SetAnim(m, obj, 0x000ED6D4u);
        }
    }
}

/* ROM 0x0ED2D8 */
int Obj_ED2D8_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0ED430 */
            Score_AddCurrent(m, 0xC8u);
            uint32_t parent = rd32(m, obj + 0x08u);
            uint8_t cnt = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
            wr8(m, parent + 0x22u, cnt);
            if (cnt != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0xC8u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Score_AddCurrent(m, 0x7D0u);
            uint32_t d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x7D0u);
            Obj_DieWithBonusDbg(m, obj, d0);
            return 0;
        }
    }

    int32_t hp = (int32_t)rd32(m, obj + 0x24u);
    if (hp <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        if (hp <= 0)
            Score_OnHit(m, obj, 0xC8u);

        uint32_t d0 = Obj_Spawn(m, obj, 0x000DC56Eu, 3u, 2u, 0u);
        if ((int32_t)d0 >= 0) {
            wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x0Cu));
            wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x0Cu));
            wr16(m, d0 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, d0 + 0x60u, rd16(m, obj + 0x60u));
        }

        uint32_t parent = rd32(m, obj + 0x08u);
        uint8_t cnt = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
        wr8(m, parent + 0x22u, cnt);
        if (cnt != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
            Obj_DieWithBonusChk(m, obj, d0);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        Score_OnHit(m, obj, 0x7D0u);
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x7D0u);
        Obj_DieWithBonus(m, obj, d0);
        return 0;
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x02u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }

    if (rd8(m, obj + 0x20u) != 0u) {
        /* 0ED330 + 0x23A = 0x0ED56A */
        if (ObjScript_CF4B8(m, obj, 0x000ED56Au) < 0)
            return -1;
    }

    if ((int16_t)rd16(m, obj + 0x3Au) <= -0x20) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint8_t st = rd8(m, obj + 0x06u);
    if (st == 0u) {
        state_0ED496(m, obj);
    } else if (st == 1u) {
        if (rd8(m, obj + 0x20u) == 0u) {
            Obj_SetScript(m, obj, 0x000ED5A2u);
            Obj_SetAnim(m, obj, 0x000ED5FCu);
            wr8(m, obj + 0x06u, 0u);
        }
    }

    Obj_AnimStep(m, obj);
    Coll_AddEnemyHazard(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}

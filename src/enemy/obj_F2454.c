#include "obj_F2454.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0F261C */
static void state_0F261C(const f3mem_t *m, uint32_t obj)
{
    static const uint16_t angles[5] = {
        0x008Cu, 0x00A0u, 0x00B4u, 0x00C8u, 0x00DCu
    };
    uint32_t x = rd32(m, obj + 0x14u) + 0x000B0000u;
    uint32_t y = rd32(m, obj + 0x18u) + 0x00100000u;

    for (int i = 0; i < 5; i++) {
        uint32_t shot = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
        if ((int32_t)shot < 0)
            break;
        wr32(m, shot + 0x14u, x);
        wr32(m, shot + 0x18u, y);
        wr16(m, shot + 0x58u, angles[i]);
        wr16(m, shot + 0x10u, 0u);
    }

    /* 0F273E: 0F2740 + 0x120 = 0x0F2860 */
    Obj_SetAnim(m, obj, 0x000F2860u);
    wr8(m, obj + 0x06u, 2u);
}

/* ROM 0x0F2454 */
int Obj_F2454_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0F25B2 */
            Score_AddCurrent(m, 0x7D0u);
            uint32_t parent = rd32(m, obj + 0x08u);
            uint8_t cnt = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
            wr8(m, parent + 0x22u, cnt);
            if (cnt != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0x7D0u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Score_AddCurrent(m, 0x4E20u);
            uint32_t d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x4E20u);
            Obj_DieWithBonusDbg(m, obj, d0);
            return 0;
        }
    }

    int32_t hp = (int32_t)rd32(m, obj + 0x24u);
    if (hp <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        if (hp <= 0)
            Score_OnHit(m, obj, 0x7D0u);

        uint32_t d0 = Obj_Spawn(m, obj, 0x000DC56Eu, 3u, 2u, 0u);
        if ((int32_t)d0 >= 0) {
            wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x18u));
            wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x10u));
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
        Score_OnHit(m, obj, 0x4E20u);
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x4E20u);
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
        /* 0F24AC + 0x2A4 = 0x0F2750 */
        if (ObjScript_CF4B8(m, obj, 0x000F2750u) < 0)
            return -1;
    }

    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    if (rd8(m, obj + 0x06u) == 1u)
        state_0F261C(m, obj);

    Obj_AnimStep(m, obj);
    Coll_AddEnemyHazard(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}

#include "obj_EE0A4.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* Common tail of the destroyed paths (0x0EDD40 / 0x0EDDB2 in the 0x0EDC38 copy) */
static void parent_count_down(const f3mem_t *m, uint32_t obj, uint32_t d0, int dbg)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
    wr8(m, a1 + 0x22u, rem);
    if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
        if (dbg) Obj_DieWithBonusDbgChk(m, obj, d0);
        else Obj_DieWithBonusChk(m, obj, d0);
        return;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if (dbg) Score_AddCurrent(m, 0x00000BB8u);
    else Score_OnHit(m, obj, 0x00000BB8u);
    d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000BB8u);
    if (dbg) Obj_DieWithBonusDbg(m, obj, d0);
    else Obj_DieWithBonus(m, obj, d0);
}

/* ROM 0x0EDC38 (same code at 0x0EE0A4, 0x0EE468, 0x0EE7D8): formation enemy
 * that fires 0xD9092 at the players. The copies differ only in the
 * animation table; cf4b8_tbl is also the aiming record for 0x0D09B8. */
int Enemy_FormationShooter(const f3mem_t *m, uint32_t obj,
                           uint32_t cf4b8_tbl, uint32_t anim_tbl)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        if ((int32_t)rd32(m, obj + 0x24u) <= 0)
            Score_OnHit(m, obj, 0x0000012Cu);
        uint32_t d0 = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
        if ((int32_t)d0 >= 0) {
            uint32_t a1 = d0;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 8u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
        parent_count_down(m, obj, d0, 0);
        return 0;
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
    if ((f7 & 0x02u) != 0u && ObjScript_CF1E4(m, obj) < 0)
        return -1;
    if (rd8(m, obj + 0x20u) != 0u && ObjScript_CF4B8(m, obj, cf4b8_tbl) < 0)
        return -1;
    Obj_ArmOnScreen(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            Obj_ResetAnimKey(m, obj);
            Obj_SetAnimByDir(m, obj, anim_tbl, rd16(m, obj + 0x58u));
            wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    case 1u:
        if (rd8(m, obj + 0x23u) == 0u) {
            if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
                break;
            Obj_AimAtPlayers(m, obj, cf4b8_tbl);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                break;
            uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)a1 < 0)
                break;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 8u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));
            wr16(m, a1 + 0x10u, 0u);
            wr8(m, obj + 0x23u, 0x1Eu);
            wr8(m, obj + 0x06u, 0x02u);
        }
        break;
    case 2u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u)
            wr8(m, obj + 0x06u, 0x01u);
        break;
    }
    default:
        return -1;
    }

    Obj_SetAnimByDir(m, obj, anim_tbl, rd16(m, obj + 0x58u));
    Obj_AnimStep(m, obj);
    Coll_AddEnemy(m, obj);
    Obj_HitFlash(m, obj);
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Score_AddCurrent(m, 0x0000012Cu);
            parent_count_down(m, obj, 0x0000012Cu, 1);
        }
    }
    return 0;
}

/* ROM 0x0EE0A4: same code as 0x0EDC38 */
int Obj_EE0A4_Tick(const f3mem_t *m, uint32_t obj)
{
    return Enemy_FormationShooter(m, obj, 0x000EDEC2u, 0x000EE394u);
}

#include "obj_EA8EE.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static int check_player_above(const f3mem_t *m, uint32_t obj, uint32_t player)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return 0;
    int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, player + 0x14u) - 7);
    if (dx < 0)
        dx = (int16_t)-dx;
    if ((uint16_t)dx > 0x1Eu)
        return 0;
    int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, player + 0x18u) + 1);
    return dy < 0;
}

/* ROM 0x0EA8EE */
int Obj_EA8EE_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Score_AddCurrent(m, 0x1388u);
            Obj_0CEF0E(m, obj);
            return 0;
        }
    }

    int32_t hp = (int32_t)rd32(m, obj + 0x24u);
    if (hp <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        if (hp <= 0)
            Score_OnHit(m, obj, 0x1388u);

        uint32_t d0 = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
        if ((int32_t)d0 >= 0) {
            wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x10u));
            wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x10u));
            wr16(m, d0 + 0x58u, 0u);
            wr16(m, d0 + 0x60u, 0u);
        }
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x02u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }

    if (rd8(m, obj + 0x20u) != 0u) {
        /* 0EA94E + 0x1A0 = 0x0EAAEE */
        if (ObjScript_CF4B8(m, obj, 0x000EAAEEu) < 0)
            return -1;
    }

    Obj_ArmOnScreen(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint32_t orb_d0 = 0u, orb_d1 = 0u;
    Obj_0CF898(m, obj, &orb_d0, &orb_d1);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + orb_d1);

    uint8_t st = rd8(m, obj + 0x06u);
    if (st == 0u) {
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            if (check_player_above(m, obj, 0x0041B190u) ||
                check_player_above(m, obj, 0x0041B21Au)) {
                Obj_SetScript(m, obj, 0x000EAB6Eu);
                wr8(m, obj + 0x06u, 1u);
            } else if (rd8(m, obj + 0x35u) != 0u) {
                Obj_SetScript(m, obj, 0x000EAB50u);
                wr8(m, obj + 0x06u, 1u);
            }
        }
    } else if (st == 1u) {
        if (rd8(m, obj + 0x35u) != 0u) {
            Obj_SetScript(m, obj, 0x000EAB50u);
        } else if (rd8(m, obj + 0x20u) == 0u) {
            Obj_SetScript(m, obj, 0x000EAB3Eu);
            wr8(m, obj + 0x06u, 0u);
        }
    }

    Obj_AnimStep(m, obj);
    Coll_AddEnemyHazard(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}

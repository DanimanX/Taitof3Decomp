#include "obj_EFF34.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static void eff34_spawn_burst(const f3mem_t *m, uint32_t obj)
{
    uint32_t d1 = rd32(m, obj + 0x14u) + 0x00280000u;
    uint32_t d2 = rd32(m, obj + 0x18u) + 0x00280000u;
    uint16_t d3 = (uint16_t)(rd16(m, obj + 0x60u) + 2u);
    uint16_t d4 = rd16(m, obj + 0x58u);

    uint32_t a1 = Obj_Spawn(m, obj, 0x000D9C88u, 9u, 2u, 0u);
    if ((int32_t)a1 < 0)
        return;
    wr32(m, a1 + 0x14u, d1);
    wr32(m, a1 + 0x18u, d2);
    wr16(m, a1 + 0x60u, d3);
    int16_t ang1 = (int16_t)(d4 - 0x28u);
    if (ang1 < 0)
        ang1 = (int16_t)(ang1 + 0x168);
    wr16(m, a1 + 0x58u, (uint16_t)ang1);

    a1 = Obj_Spawn(m, obj, 0x000D9C88u, 9u, 2u, 0u);
    if ((int32_t)a1 < 0)
        return;
    wr32(m, a1 + 0x14u, d1);
    wr32(m, a1 + 0x18u, d2);
    wr16(m, a1 + 0x60u, d3);
    uint16_t ang2 = (uint16_t)(d4 + 0x28u);
    if (ang2 >= 0x168u)
        ang2 = (uint16_t)(ang2 - 0x168u);
    wr16(m, a1 + 0x58u, ang2);
}

/* ROM 0x0EFF34: enemy mini-boss/pod tick */
int Obj_EFF34_Tick(const f3mem_t *m, uint32_t obj)
{
    static const uint32_t k_rec_tbl[6] = {
        0x000F029Eu, 0x000F02D6u, 0x000F030Eu,
        0x000F0346u, 0x000F037Eu, 0x000F03B6u
    };
    int goto_die = 0;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0xBB8u);
        eff34_spawn_burst(m, obj);
        goto_die = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_die = 1;
    }

    if (goto_die) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x28u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x28u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
        uint32_t p = rd32(m, obj + 0x08u);
        uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
        wr8(m, p + 0x22u, cnt);
        if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
            Obj_0CEE48(m, obj);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x18u));
        Obj_0D0E04(m, obj, 0x7530u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x7530u);
        Obj_0CEE48(m, obj);
        return 0;
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
    if ((f7 & 0x02u) != 0u) {
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        uint8_t idx = rd8(m, obj + 0x22u);
        if (idx >= 6u)
            return -1;
        if (ObjScript_CF4B8(m, obj, k_rec_tbl[idx]) < 0)
            return -1;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        Obj_AimAtPlayers(m, obj, 0x000F029Eu);
        if (rd8(m, obj + 0x23u) != 0u)
            return -1;
        break;
    case 1u: {
        uint16_t r = Obj_Next4Way(m);
        Obj_StartScript(m, obj, ((int8_t)(uint8_t)r < 0) ? 0x000F045Cu : 0x000F043Eu);
        wr8(m, obj + 0x06u, 2u);
        break;
    }
    case 2u: {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x < -0x50 || y < -0x50 || x > 0x140 || y > 0xE8)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        break;
    }
    default:
        return -1;
    }

    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    Obj_SetAnimByDir(m, obj, 0x000F047Au, rd16(m, obj + 0x58u));
    Obj_StepScript(m, obj);
    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            return -1;
        }
    }
    return 0;
}

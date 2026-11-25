#include "obj_E739E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static void e739e_case1(const f3mem_t *m, uint32_t obj)
{
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (y < -0x20 || y > 0xC7)
        return;
    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    for (int i = 0; i < 2; i++) {
        uint32_t p = players[i];
        if ((int16_t)rd16(m, p + 0x4Cu) >= 0) {
            int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) - 0x0C);
            if (dx < 0) dx = (int16_t)(-dx);
            if ((uint16_t)dx <= 0x0Au)
                goto fire;
        }
    }
    if ((uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3) < (uint8_t)Obj_Next4Way(m))
        return;
fire:
    wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) ^ 0x01u));
    Obj_StartBehaviour(m, obj, (rd8(m, obj + 0x22u) & 0x01u) ? 0x000E7662u : 0x000E75F6u);
    wr8(m, obj + 0x06u, 2u);
}

static void e739e_case2(const f3mem_t *m, uint32_t obj)
{
    Obj_StepScript(m, obj);
    if (rd16(m, obj + 0x4Cu) != 0u) {
        int16_t d1 = (int16_t)rd16(m, obj + 0x68u);
        if (d1 < 0)
            return;
        (void)Obj_Spawn(m, obj, (d1 > 0) ? 0x000E78A6u : 0x000E76CEu, 9u, 2u, 0u);
        return;
    }
    uint32_t a1 = (rd8(m, obj + 0x22u) != 0u) ? 0x000E75ECu : 0x000E75E2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
    wr8(m, obj + 0x23u, 0x1Eu);
    wr8(m, obj + 0x06u, 3u);
}

/* ROM 0x0E739E: parent-attached turret enemy */
int Obj_E739E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, parent + 0x06u) == 0u)
        goto dead;
    if (rd8(m, obj + 0x06u) == 0u) {
        Obj_DrawSprite(m, parent);
        return 0;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u)
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x0190u);
        goto dead;
    }
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0019u));
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x000Au));
    switch (rd8(m, obj + 0x06u)) {
    case 1u: e739e_case1(m, obj); break;
    case 2u: e739e_case2(m, obj); break;
    case 3u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u)
            wr8(m, obj + 0x06u, 1u);
        break;
    }
    default:
        return -1;
    }
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, parent);
    return 0;

dead:
    if (rd8(m, obj + 0x06u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        if ((int16_t)rd16(m, obj + 0x14u) >= -0x50) {
            uint32_t c = Obj_Spawn(m, obj, 0x000DC358u, 3u, 2u, 0u);
            if ((int32_t)c >= 0) {
                wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x10u));
                wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x20u));
                wr16(m, c + 0x58u, 0u);
                wr16(m, c + 0x60u, 0u);
            }
        }
    }
    if (rd8(m, parent + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, parent);
    return 0;
}

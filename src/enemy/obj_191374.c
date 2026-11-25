#include "obj_191374.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

/* ROM 0x193E6C: knock back the object the parent hit (+0x36) when armed */
static void sub_193E6C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 1u)
        return;
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k == 0u || k == 4u || k == 5u || k == 6u)
        return;
    if ((int32_t)rd32(m, a1 + 0x48u) >= 0)
        wr32(m, a1 + 0x48u, (uint32_t)(-(int32_t)rd32(m, a1 + 0x48u)));
    uint16_t d0 = Obj_RandRange(m, 4u);
    wr16(m, a1 + 0x4Cu, (uint16_t)(rd16(m, a1 + 0x4Cu) + d0));
    wr8(m, a1 + 0x1Cu, (uint8_t)(rd8(m, a1 + 0x1Cu) | 1u));
    wr8(m, a1 + 0x1Du, 4u);
    wr32(m, a1 + 0x28u, 0u);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x000F));
}

/* ROM 0x191374: follower -- copy the parent's position; when the parent is in
 * anim 5 register as a collider, and spawn the 0x1802A4 shot on one pose */
int Obj_191374_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x14u, rd16(m, a4 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a4 + 0x18u));

    if (rd16(m, a4 + 0x40u) == 5u) {
        uint32_t d0 = rd32(m, a4 + 0x10u);
        uint32_t a1 = 0x191456u;
        for (int i = 0; i < 4; i++, a1 += 12u) {
            if (rd32(m, a1) == d0) {
                wr32(m, obj + 0x2Cu, rd32(m, a1 + 4u));
                wr32(m, obj + 0x30u, rd32(m, a1 + 8u));
                break;
            }
        }
        if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
            uint32_t l = rd32(m, F3_A5 + 0x08FEu);
            wr32(m, l, obj);
            wr32(m, F3_A5 + 0x08FEu, l + 4u);
            l = rd32(m, F3_A5 + 0x0906u);
            wr32(m, l, obj);
            wr32(m, F3_A5 + 0x0906u, l + 4u);
            sub_193E6C(m, obj);
            wr8(m, obj + 0x35u, 0u);
        }
    }

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x08u) != 0u &&
        rd32(m, a4 + 0x10u) == 0x3D8E4u && rd16(m, a4 + 0x3Eu) == 0x0Au) {
        uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) - 0x75u);
        uint16_t y = (uint16_t)(rd16(m, obj + 0x54u) - 0x17u);
        uint8_t ang = Obj_AimAtPlayer17AF9A(m, rd32(m, F3_A5 + 0x1D28u), x, y);
        Obj_Spawn(m, obj, 0x1802A4u, 9u, 2u, 0x8AE90000u | ang);
    }
    wr32(m, obj, 0x191374u);
    return 0;
}

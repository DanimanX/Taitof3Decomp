#include "obj_17F5DA.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17F5DA: boss/enemy rapid particle burst & multi-spawn phase */
int Obj_17F5DA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);

    uint8_t rx = (uint8_t)Obj_RandRange(m, 0x40u);
    uint8_t ry = (uint8_t)Obj_RandRange(m, 0x40u);
    uint32_t arg = ((uint32_t)rx << 24) | ((uint32_t)ry << 16);
    (void)Obj_Spawn(m, obj, 0x0017F7CEu, 9u, 2u, arg);

    int16_t t = (int16_t)(rd16(m, obj + 0x64u) - 1u);
    wr16(m, obj + 0x64u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0017F5DAu);
        return 0;
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0023)); /* 0x003B66 */
    if (rd16(m, F3_A5 + 0x4B38u) == 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t d1 = 0xFFE00000u;
    for (int i = 0; i < 10; i++) {
        (void)Obj_Spawn(m, obj, 0x0017F8B0u, 5u, 2u, d1);
        d1 += 0xFFE00000u;
    }
    wr16(m, obj + 0x1Cu, 8u);
    uint32_t a1 = 0x0017F9ECu;
    uint16_t anim_idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;
    wr16(m, obj + 0x64u, 0x003Cu);
    wr16(m, obj + 0x1Eu, 0u);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0022));
    wr32(m, obj + 0x00u, 0x0017F6C8u);
    return 0;
}

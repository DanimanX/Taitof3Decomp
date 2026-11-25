#include "obj_17F260.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17F260: boss sub-part / laser pod delay & spawn init */
int Obj_17F260_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0017F260u);
        return 0;
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x002A));
    wr8(m, obj + 0x0Eu, 8u);
    Obj_PolarPlace(m, obj);
    Obj_184888(m, obj, 0x0017F2FAu);
    uint32_t a1 = 0x0017F310u;
    uint16_t anim_idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;
    uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 7u);
    wr32(m, obj + 0x62u, child);
    if ((int32_t)child <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t spr = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x16u, (uint16_t)((int16_t)rd16(m, spr + 2u) << 3));
    wr16(m, obj + 0x1Au, (uint16_t)((int16_t)rd16(m, spr) << 3));
    wr16(m, obj + 0x58u, 0u);
    wr16(m, obj + 0x5Au, 0u);
    wr8(m, obj + 0x34u, 2u);
    wr32(m, obj + 0x00u, 0x0017C360u);
    return 0;
}

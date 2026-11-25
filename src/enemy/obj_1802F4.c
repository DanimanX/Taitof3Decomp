#include "obj_1802F4.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1802F4: countdown timer (+0x06), then init + arm 0x17C05E */
int Obj_1802F4_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x001802F4u);
        return 0;
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x002A));
    Obj_184888(m, obj, 0x00180444u);
    uint32_t a1 = 0x0018045Au;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;
    wr32(m, obj + 0x44u, 0x00180462u);
    wr16(m, obj + 0x48u, rd16(m, 0x00180462u + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x00u, 0x0017C05Eu);
    return 0;
}

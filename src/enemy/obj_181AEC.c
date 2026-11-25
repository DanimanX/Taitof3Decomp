#include "obj_181AEC.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x181AEC: spawn enemy if active player exists and in bounds */
int Obj_181AEC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x4Cu, tgt);
    if (tgt == 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_PolarPlace(m, obj);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0011));
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < -0x10 || x > 0x14F || y < -0x10 || y > 0xF7) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_184888(m, obj, 0x0017DAB4u);
    uint32_t a1 = 0x0017DACA;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;
    wr32(m, obj + 0x44u, 0x00181BA8u);
    wr16(m, obj + 0x48u, rd16(m, 0x00181BA8u + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x0Cu, 0x00181B76u);
    wr32(m, obj + 0x00u, 0x0017C14Cu);
    return 0;
}

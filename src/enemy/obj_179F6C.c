#include "obj_179F6C.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x179F6C: boss projectile init (cap 0x1E active at A5+0x4B24) */
int Obj_179F6C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));
    uint32_t a4 = rd32(m, obj + 0x08u);
    int16_t ox = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t oy = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + ox));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + oy));
    wr32(m, obj + 0x44u, 0x0017A002u);
    wr16(m, obj + 0x48u, rd16(m, 0x0017A002u + 4u));
    wr16(m, obj + 0x4Au, 0u);
    uint32_t a1 = 0x00179FCEu;
    uint16_t d0 = rd16(m, a1); a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0012));
    wr32(m, obj + 0x00u, 0x00179C6Eu);
    return 0;
}

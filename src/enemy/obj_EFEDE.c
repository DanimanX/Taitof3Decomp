#include "obj_EFEDE.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0EFEDE: enemy init */
int Obj_EFEDE_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x000F029Eu);
    wr8(m, obj + 0x22u, rd8(m, obj + 0x10u));
    Obj_ClassColour(m, obj);
    wr32(m, obj + 0x44u, 0x000F0426u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    wr32(m, rd32(m, obj + 0x0Cu) + 0x28u, 0u);
    wr32(m, obj + 0x68u, 0x000F075Au);
    int16_t d0 = (int16_t)(rd16(m, obj + 0x36u) - 0x00B4u);
    if (d0 < 0)
        d0 = (int16_t)(d0 + 0x0168u);
    wr16(m, obj + 0x58u, (uint16_t)d0);
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000EFF34u);
    return 0;
}

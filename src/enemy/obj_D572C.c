#include "obj_D572C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"


/* ROM 0x0CBF7A */
static void sub_0CBF7A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x28740u);
    wr32(m, obj, 0xCBF8Au);
}

/* ROM 0x0D572C */
int Obj_D572C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj, 0xD572Cu);
        return 0;
    }
    uint32_t a1 = rd32(m, rd32(m, obj + 0x08u) + 0x50u);
    int gt = (int16_t)rd16(m, a1 + 0x18u) > 0x6C;
    uint16_t d0 = gt ? 0xFFF0u : 0x00E8u;
    wr8(m, obj + 0x64u, gt ? 0x02u : 0x00u);
    wr32(m, obj + 0x18u, 0u);
    wr16(m, obj + 0x18u, d0);
    wr32(m, obj + 0x50u, 0xD576Cu);
    sub_0CBF7A(m, obj);
    return 0;
}

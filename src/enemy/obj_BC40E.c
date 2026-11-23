#include "obj_BC40E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBC40E */
int Obj_BC40E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC40E: lea (0x3C,PC),A1 -> 0x0BC40C */
    uint32_t a1 = 0x000BC410u + 0x3Cu;
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x44u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x0086u);
    /* 0x0BC4A0 */
    wr32(m, obj, 0x000BC4A8u);
    return 0;
}

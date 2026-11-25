#include "obj_18048A.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x18048A: polar place, bounds check, spawn 0x17C856, arm 0x1804DA */
int Obj_18048A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_PolarPlace(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < -0x10 || x > 0x14F || y < -0x10 || y > 0xF7) {
        Obj_Free(m, obj);
        return 0;
    }
    (void)Obj_Spawn(m, obj, 0x0017C856u, 11u, 1u, 0u);
    wr16(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x001804DAu);
    return 0;
}

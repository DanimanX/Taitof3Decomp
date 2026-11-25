#include "obj_17F006.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17F006: polar placement + spawn charge ring */
int Obj_17F006_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_PolarPlace(m, obj);
    (void)Obj_Spawn(m, obj, 0x0017CCBEu, 0x0Bu, 1u, 0x00140006u);
    wr16(m, obj + 0x06u, 0x0023u);
    wr32(m, obj + 0x00u, 0x0017F038u);
    return 0;
}

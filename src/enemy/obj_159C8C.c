#include "obj_159C8C.h"
#include "../objects/obj_common.h"

/* ROM 0x159C8C: palette ramp transition step 1 */
int Obj_159C8C_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x00159C8Cu);
        return 0;
    }

    Pal_QueueRamp(m, 0x00020340u, 0x0015A492u, 0x0880u, 0x002Fu, 0x0020u);
    Pal_QueueRamp(m, 0x00020440u, 0x0015A492u, 0x0980u, 0x002Fu, 0x0020u);
    Pal_QueueRamp(m, 0x0015A512u, 0x0015A412u, 0x0000u, 0x0017u, 0x0001u);

    wr16(m, obj + 0x06u, 0x0018u);
    wr32(m, obj + 0x00u, 0x00159D22u);
    return 0;
}

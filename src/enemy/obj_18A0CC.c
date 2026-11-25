#include "obj_18A0CC.h"
#include "../objects/obj_common.h"

/* ROM 0x18A0CC: queue two palette ramps and arm 0x18A12C */
int Obj_18A0CC_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)Pal_QueueRamp(m, 0x0040B42Eu, 0x0014E15Au, 0x0B80u, 0x0040u, 0x0010u);
    (void)Pal_QueueRamp(m, 0x0014E29Au, 0x0014E2DAu, 0x0C80u, 0x0080u, 0x0010u);
    wr16(m, obj + 0x06u, 0x0040u);
    wr32(m, obj + 0x00u, 0x0018A12Cu);
    return 0;
}

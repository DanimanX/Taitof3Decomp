#include "obj_C8208.h"

/* ROM 0x0C8208: arm a 0x14-frame wait and hand over to 0x0C8218. The `beq`
 * at 0x0C820E follows move.w #0x14 (Z clear), so the 0x0C821E reset path
 * is not reachable from this entry. */
int Obj_C8208_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x0014u);
    wr32(m, obj + 0x00u, 0x000C8218u);
    return 0;
}

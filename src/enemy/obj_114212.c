#include "obj_114212.h"

/* ROM 0x114212: thunk into the 0x1140F4 chain. It copies the parent-facing
 * byte +0x4F into the sprite index +0x1D, arms the +0x6 timer at 0x14 and
 * parks the slot on 0x114108, which is where the phase body starts. */
int Obj_114212_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1140F4 */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x06u, 0x0014u);
    wr32(m, obj + 0x00u, 0x00114108u);
    return 0;
}

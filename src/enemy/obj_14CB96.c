#include "obj_14CB96.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x14CB96: countdown +0x06; on expiry clear two 8-byte palette/scroll
 * pairs at A5+0x4214 / A5-0x5D98 and A5+0x421C / A5-0x5D90, queue 0x20-colour
 * ramp from 0x00198E to 0x0203C0 (d0=0x900, frames=0xB4), and free self. */
int Obj_14CB96_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)rd16(m, obj + 0x06u) - 1;
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0014CB96u);
        return 0;
    }

    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0u);
    Mem_WritePair64(m, F3_A5 + 0x421Cu, F3_A5 - 0x5D90u, 0u, 0u);
    (void)Pal_QueueRamp(m, 0x0000198Eu, 0x000203C0u, 0x0900u, 0x00B4u, 0x0020u);
    Obj_Free(m, obj);
    return 0;
}

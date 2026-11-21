#include "obj_CB412.h"

/* ROM 0x0CB412: wait for A5+0x1D58 bit 1 */
int Obj_CB412_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        wr32(m, obj + 0x00u, 0x000CB412u);
    else
        wr32(m, obj + 0x00u, 0x000CB422u);
    return 0;
}

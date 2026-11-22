#include "obj_114092.h"
#include "../objects/obj_pool.h"

/* ROM 0x114092 */
int Obj_114092_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x114092: timer--; still running -> re-arm at 0x11408A, else free */
    uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, c);
    if ((int16_t)c > 0) {
        wr32(m, obj + 0x00u, 0x00114092u);       /* 0x11408A */
        return 0;
    }
    Obj_Free(m, obj);                            /* 0x11409A */
    return 0;
}

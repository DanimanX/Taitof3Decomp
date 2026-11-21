#include "obj_12EF24.h"

/* ROM 0x12EF24: follow the +0x48 chain from (A5+0x32AA) to the node with +0x22 == 1 */
int Obj_12EF24_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);
    for (int guard = 0;; guard++) {
        if (guard > 0x10000)
            return -1; /* ROM would hang: no matching node */
        a1 = rd32(m, a1 + 0x48u);
        if (rd16(m, a1 + 0x22u) == 1u)
            break;
    }
    wr32(m, obj + 0x0Cu, a1);
    wr32(m, obj + 0x00u, 0x0012EF40u);
    return 0;
}

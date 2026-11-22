#include "obj_13029E.h"
#include "../objects/obj_pool.h"

/* ROM 0x13029E */
int Obj_13029E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {                 /* 0x13029E btst.b #0 */
        Obj_Free(m, obj);                                  /* 0x1302EA */
        return 0;
    }
    /* 0x1302A8: count = arg */
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x06u, (uint16_t)d0);
    /* 0x1302B0 */
    wr32(m, obj + 0x00u, 0x001302B8u);
    return 0;
}

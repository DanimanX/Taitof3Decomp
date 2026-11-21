#include "obj_D7570.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D7570: frees obj+0x50 (when positive, i.e. not -1, and still owned by
 * this object via its +0x08 parent) and then itself. */
int Obj_D7570_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x50u);
    if ((int32_t)d0 > 0) {
        uint32_t a1 = d0;
        if (rd32(m, a1 + 0x08u) == obj)
            Obj_Free(m, a1);
    }
    Obj_Free(m, obj);
    return 0;
}

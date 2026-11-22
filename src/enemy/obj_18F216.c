#include "obj_18F216.h"
#include "../objects/obj_pool.h"

/* ROM 0x18F216: countdown at +0x06; while still positive just re-arm the
 * same tick, else free the object. */
int Obj_18F216_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18F216 subq.w #1,(0x6,A0) */
    int16_t d0 = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)d0);
    if (d0 > 0) {
        /* 0x18F20E move.l #0x18F216,(A0) */
        wr32(m, obj + 0x00u, 0x0018F216u);
        return 0;
    }
    /* 0x18F21C: Obj_Free(A0) */
    Obj_Free(m, obj);
    return 0;
}

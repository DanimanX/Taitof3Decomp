#include "obj_18B3AE.h"
#include "../objects/obj_pool.h"

/* ROM 0x18B3AE: countdown at +0x06; while still positive just re-arm the
 * same tick, else free the object. */
int Obj_18B3AE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B3AE subq.w #1,(0x6,A0) */
    int16_t d0 = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)d0);
    if (d0 > 0) {
        /* 0x18B3A6 move.l #0x18B3AE,(A0) */
        wr32(m, obj + 0x00u, 0x0018B3AEu);
        return 0;
    }
    /* 0x18B3B4: Obj_Free(A0) */
    Obj_Free(m, obj);
    return 0;
}

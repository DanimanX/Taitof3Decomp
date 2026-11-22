#include "obj_17EDA4.h"
#include "../objects/obj_pool.h"

/* ROM 0x17C78E: free the object (shared tail of 0x17DD46 / 0x17EDA4). */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17EDA4: countdown at +0x06; while still positive just re-arm the
 * same tick, else free the object. */
int Obj_17EDA4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17EDA4 subq.w #1,(0x6,A0) */
    int16_t d0 = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)d0);
    if (d0 > 0) {
        /* 0x17ED9C move.l #0x17EDA4,(A0) */
        wr32(m, obj + 0x00u, 0x0017EDA4u);
        return 0;
    }
    /* 0x17EDAA bra.w 0x17C78E */
    lab_17C78E(m, obj);
    return 0;
}

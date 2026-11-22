#include "obj_17AADA.h"
#include "../objects/obj_pool.h"

/* ROM 0x0017AAD2: re-arm the tick vector with 0x17AADA. */
static void Obj_17AADA_SetVector(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0017AADAu);
}

/* ROM 0x17AADA: wait +0x6 frames, then free the object. */
int Obj_17AADA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17AADA: subq.w #1,(0x6,A0); bgt.b 0x17AAD2 */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_17AADA_SetVector(m, obj);
        return 0;
    }

    /* 0x17AAE0: move.l A0,-(SP); jsr 0x0105EC; rts */
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}

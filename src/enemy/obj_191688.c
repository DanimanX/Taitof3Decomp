#include "obj_191688.h"
#include "../objects/obj_pool.h"

/* ROM 0x00191680: re-arm the tick vector with 0x191688. */
static void Obj_191688_SetVector(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00191688u);
}

/* ROM 0x191688: wait +0x6 frames, then free the object. */
int Obj_191688_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x191688: subq.w #1,(0x6,A0); bgt.b 0x191680 */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_191688_SetVector(m, obj);
        return 0;
    }

    /* 0x19168E: move.l A0,-(SP); jsr 0x0105EC; rts */
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}

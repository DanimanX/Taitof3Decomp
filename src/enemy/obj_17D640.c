#include "obj_17D640.h"
#include "../objects/obj_pool.h"

/* ROM 0x17D640: the spawned-child wait tail. +0x06 counts down while positive;
 * once it reaches zero the object releases its slot (0x17C78E). */
int Obj_17D640_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D640: subq.w #1,(0x6,A0); bgt.b -> 0x17D638 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        wr32(m, obj + 0x00u, 0x0017D640u);
        return 0;
    }

    /* 0x17D646: bra.w 0x17C78E */
    Obj_Free(m, obj);
    return 0;
}

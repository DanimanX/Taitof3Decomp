#include "obj_10A42C.h"
#include "../objects/obj_common.h"

/* ROM 0x10A42C */
int Obj_10A42C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10A42C: tst.b (0x22,A0) / beq.l 0x0CEE74 */
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10A436 */
    return 0;
}

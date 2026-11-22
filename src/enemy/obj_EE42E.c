#include "obj_EE42E.h"
#include "../objects/obj_common.h"

/* ROM 0x0EE42E: if the +0x22 hit/timer byte is clear the handler is replaced
 * by the dying chain (0CEE74 'beq.l', tail call); nothing else is touched. */
int Obj_EE42E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0EE42E tst.b (0x22,A0); 0x0EE432 beq.l 0x0CEE74 */
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0EE438 rts */
    return 0;
}

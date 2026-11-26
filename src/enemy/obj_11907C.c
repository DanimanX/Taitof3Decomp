#include "obj_11907C.h"
#include "../objects/obj_common.h"

/* ROM 0x11907C: reset fields, set +0x22 to 0xff, then hand over to 0x119090. */
int Obj_11907C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11907C: jsr 0x000cbd86 */
    Obj_ResetFields(m, obj);
    /* 0x119082: move.w #0xff,(0x22,A0) */
    wr16(m, obj + 0x22u, 0x00ffu);
    /* 0x119088: move.l #0x119090,(A0) */
    wr32(m, obj + 0x00u, 0x00119090u);
    return 0;
}

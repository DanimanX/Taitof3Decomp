#include "obj_1116D4.h"
#include "../objects/obj_common.h"

int Obj_1116D4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1116D4: jsr 0x000cbd86.l ; fall through to 0x1116DA */
    Obj_ResetFields(m, obj);
    /* 0x1116DA: move.l #0x1116e2,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x001116E2u);
    return 0;
}

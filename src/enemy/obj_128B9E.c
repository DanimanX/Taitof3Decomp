#include "obj_128B9E.h"

/* ROM 0x128B9E: while the object is still in group+0x4C (TST.W leaves Z clear)
 * it shows no sprite (+0x06 = 0x78) and restarts at LAB_00128baa; once +0x4C
 * has run out it re-arms at 0x128B96, which parks it on itself. */
int Obj_128B9E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x4Cu) == 0u) {
        /* 0x128B96: beq -> move.l #0x128b9e,(A0) */
        wr32(m, obj + 0x00u, 0x00128B9Eu);
        return 0;
    }

    /* 0x128BA4: move.w #0x78,(0x6,A0) */
    wr16(m, obj + 0x06u, 0x0078u);

    /* LAB_00128baa */
    wr32(m, obj + 0x00u, 0x00128BB2u);
    return 0;
}

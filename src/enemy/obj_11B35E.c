#include "obj_11B35E.h"
#include "../objects/obj_pool.h"

/* ROM 0x11B35E: advance the two scroll/offset globals, and when bit 3 of the
 * global byte at A5-0x144B is clear keep ticking, else free the object. */
int Obj_11B35E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11B35E addi.l #0x3000,(0x30D2,A5) */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x3000u);
    /* 0x11B366 addi.l #0x6000,(0x30D6,A5) */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + 0x6000u);
    /* 0x11B36E btst.b #3,(-0x144B,A5); beq 0x11B356 */
    if (!(rd8(m, F3_A5 - 0x144Bu) & 0x08u)) {
        /* 0x11B356 move.l #0x11B35E,(A0) */
        wr32(m, obj + 0x00u, 0x0011B35Eu);
        return 0;
    }
    /* 0x11B376: Obj_Free(A0) */
    Obj_Free(m, obj);
    return 0;
}

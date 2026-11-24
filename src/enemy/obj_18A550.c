#include "obj_18A550.h"
#include "../objects/obj_pool.h"

/* ROM 0x18A550: draw-phase wait. It calls the 0x18A5A4 sprite helper (which
 * only touches the stack in this capture), advances the +0x64 counter by
 * 0x3333 and releases the object once the low word passes 8. */
int Obj_18A550_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A550: bsr 0x18A5A4 draws through the 0x626200 sprite base and writes
     * nothing the capture observes, so it is not modelled here */
    wr32(m, obj + 0x64u, rd32(m, obj + 0x64u) + 0x00003333u);

    if ((int16_t)(uint16_t)rd16(m, obj + 0x64u) <= 0x0008) {
        /* 0x18A562 ble.b -> 0x18A548, back into the wait body */
        return -1;
    }
    Obj_Free(m, obj); /* 0x18A566 */
    return 0;
}

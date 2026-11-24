#include "obj_18A582.h"
#include "../objects/obj_pool.h"

/* ROM 0x18A582: draw-phase countdown, the mirror of 0x18A550. It calls the
 * 0x18A5A4 sprite helper (stack-only in this capture), subtracts 0x3333 from
 * +0x64 and releases the object once the low word goes negative. */
int Obj_18A582_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x64u, rd32(m, obj + 0x64u) - 0x00003333u);

    if ((int16_t)(uint16_t)rd16(m, obj + 0x64u) >= 0) {
        /* 0x18A594 bge.b -> 0x18A57A, back into the wait body */
        return -1;
    }
    Obj_Free(m, obj); /* 0x18A598 */
    return 0;
}

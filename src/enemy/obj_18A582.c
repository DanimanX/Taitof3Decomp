#include "obj_18A582.h"
#include "obj_18A550.h"
#include "../objects/obj_pool.h"

/* ROM 0x18A582: fade out, the mirror of 0x18A550. Mask, +0x64 -= 0x3333;
 * re-arm (0x18A57A) while its high word is >= 0, else release the object. */
int Obj_18A582_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_18A5A4(m, obj);
    wr32(m, obj + 0x64u, rd32(m, obj + 0x64u) - 0x00003333u);
    if ((int16_t)rd16(m, obj + 0x64u) >= 0) {
        wr32(m, obj + 0x00u, 0x0018A582u);              /* 0x18A57A */
        return 0;
    }
    Obj_Free(m, obj);                                   /* 0x18A598 */
    return 0;
}

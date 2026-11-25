#include "obj_E76CE.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"

/* ROM 0x0E76CE: init of the 0x0E76FC rider. Stats from 0x0EABAA; it copies
 * the parent's +0x22 side flag and takes the 0x0E773E script, or 0x0E77F2
 * when the flag is set. */
int Obj_E76CE_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x000EABAAu);               /* lea (0x34DA,PC) */
    uint32_t a1 = 0x000E773Eu;                      /* lea (0x64,PC) */
    uint8_t side = rd8(m, rd32(m, obj + 0x08u) + 0x22u);
    wr8(m, obj + 0x22u, side);
    if (side != 0u)
        a1 = 0x000E77F2u;                           /* lea (0x106,PC) */
    Obj_SetAnim(m, obj, a1);                        /* 0x0D0678 */
    wr32(m, obj + 0x00u, 0x000E76FCu);
    return 0;
}

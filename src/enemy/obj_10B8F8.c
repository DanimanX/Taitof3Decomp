#include "obj_10B8F8.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x10B8F8: enemy init -- stats 0x10BC12, class colour, script
 * 0x10BC32, behaviour 0x10BC50, phase 1, +0x34 bit 1 off / bit 2 on,
 * +0x07 bit 4 on; arms 0x10B93C. */
int Obj_10B8F8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x0010BC12u);
    Obj_InitPalette(m, obj);                        /* 0x0CEE2C */
    Obj_SetScript(m, obj, 0x0010BC32u);             /* 0x0CF1D8 */
    Obj_SetAnim(m, obj, 0x0010BC50u);               /* 0x0D0678 */
    wr8(m, obj + 0x06u, 0x01u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr32(m, obj + 0x00u, 0x0010B93Cu);
    return 0;
}

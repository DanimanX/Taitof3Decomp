#include "obj_11AEBE.h"

/* ROM 0x11AEBE: clear the position pair (+0x14/+0x18), then continue at
 * LAB_0011aec6. */
int Obj_11AEBE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, 0u);   /* clr.l (0x14,A0) */
    wr32(m, obj + 0x18u, 0u);   /* clr.l (0x18,A0) */

    /* LAB_0011aec6 */
    wr32(m, obj + 0x00u, 0x0011AECEu);
    return 0;
}

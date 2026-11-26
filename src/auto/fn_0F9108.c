/* ROM 0x0F9108: explosion/spark emitter initializer (A0 = object).
 * Seeds enemy HP/dmg/hitboxes from 0x0F824C (0x0CEDE4), sets +0x21 = 0x14,
 * and arms 0x0F9120. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F9108(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x000F824Cu;                          /* 0x0F9108 */

    Obj_0CEDE4(m, obj, a1);                             /* 0x0F910C */
    r->a[1] = a1 + 0x1Cu;

    wr8(m, obj + 0x21u, 0x14u);                         /* 0x0F9112 */
    wr32(m, obj, 0x000F9120u);                          /* 0x0F9118 */
    return 0;
}

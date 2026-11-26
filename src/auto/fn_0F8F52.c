/* ROM 0x0F8F52: enemy/effect initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0F824C (0x0CEDE4), installs behaviour 0x0F8FAC
 * (0x0D0678), and arms 0x0F8F6E. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F8F52(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000F824Cu);                    /* 0x0F8F52..0x0F8F56 */

    uint32_t a1 = 0x000F8FACu;                          /* 0x0F8F5C */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F8F60 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    wr32(m, obj, 0x000F8F6Eu);                          /* 0x0F8F66 */
    return 0;
}

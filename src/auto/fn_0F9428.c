/* ROM 0x0F9428: enemy initializer (A0 = object).
 * Seeds HP/hitboxes from the template at 0x0F824C (0x0CEDE4 = the body of
 * 0x0F8F52's seed), installs the behaviour record 0x0F943A (0x0D0678, which
 * also leaves D0 = the record's first frame word 0x0001 and A1 = record + 8),
 * and arms 0x0F8F6E through the 0x0F8F60 tail. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F9428(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000F824Cu);                    /* 0x0F9428..0x0F942C */

    uint32_t a1 = 0x000F943Au;                          /* 0x0F9432 lea (0x6,PC) */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F9436 -> 0x0F8F60 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    wr32(m, obj, 0x000F8F6Eu);                          /* 0x0F8F66 */
    return 0;
}
